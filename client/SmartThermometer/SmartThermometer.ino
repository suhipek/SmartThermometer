#include <Adafruit_MLX90614.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET    -1
#define SERVER_IP   "47.100.56.128"
#define UP      12
#define DOWN      13
#define MID     14
#define TEMP_REFRESH_FREQ 500

Adafruit_SSD1306  display( SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET );
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

WiFiClient  client;

uint8_t MAC_array_STA[6];
char  MAC_char_STA[18];
String  GetUrl;
String  response;

String  uname[4] = { "aaa", "bbb", "ccc", "ddd" };
String  ustatus[4] = { "F", "F", "F", "F" };
bool  isChosen[4] = { true, false, false, false };
uint8_t choseP    = 0;            /*选中的用户是谁 */
bool  atPage    = false;        /* 是否在测温页面中 */
float currTemp  = 0;            /* 当前测温 */
float temperature[4]  = { 0 };        /* 从服务器获取到的温度 */
int lastRefresh = 0;
int currTime  = 0;

bool needRefreshTemp = false;
bool needRefreshHome = false;
bool needSubmit = false;

void refreshHomePage();


/* 网络相关函数 */
bool getConfig()
{
  HTTPClient  http;
  http.begin( client, "http://" SERVER_IP "/get_config" ); /* HTTP */
  http.addHeader( "Content-Type", "application/json" );
  StaticJsonDocument<200> doc, responseDoc;
  doc['mac'] = MAC_char_STA;
  char myDoc[measureJson( doc ) + 1];
  serializeJson( doc, myDoc, measureJson( doc ) + 1 );
  Serial.println( myDoc );
  int httpCode = http.POST( myDoc );

  if ( httpCode > 0 )
  {
    if ( httpCode == HTTP_CODE_OK )
    {
      String response = http.getString();
      http.end();
      DeserializationError error = deserializeJson( responseDoc, response );
      if ( !error )
      {
        for ( int i = 0; i <= 3; i++ )
        {
          uname[i]  = responseDoc["name"][i].as<String>();
          ustatus[i]  = responseDoc["status"][i].as<String>();
          temperature[i]  = responseDoc["temp"][i].as<float>();
        }
        return(true);
      }
    }
  } else {
    Serial.printf( "[HTTP] POST... failed, error: %s\n", http.errorToString( httpCode ).c_str() );
  }

  http.end();
}


bool submitTemp()                                                       /* 提交温度 */
{
  HTTPClient  http;
  http.begin( client, "http://" SERVER_IP "/submit_temp/" );      /* HTTP */
  http.addHeader( "Content-Type", "application/json" );
  StaticJsonDocument<200> doc;
  doc['name'] = uname[choseP];
  doc['mac']  = MAC_char_STA;
  doc['temp'] = currTemp;
  char myDoc[measureJson( doc ) + 1];
  serializeJson( doc, myDoc, measureJson( doc ) + 1 );
  Serial.println( myDoc );
  int httpCode = http.POST( myDoc );

  if ( httpCode > 0 )
  {
    if ( httpCode == HTTP_CODE_OK && http.getString() == "success" )
    {
      http.end();
      return(true);
    }
  } else {
    Serial.printf( "[HTTP] POST... failed, error: %s\n", http.errorToString( httpCode ).c_str() );
  }

  http.end();
}


void refreshTemp()              /* 刷新测温页面 */
{
  currTemp = mlx.readObjectTempC() + 3.3;
  display.clearDisplay();
  display.setCursor( 25, 10 );
  display.setTextColor( WHITE );
  display.print( uname[choseP] );
  display.setCursor( 25, 30 );
  display.setTextSize( 2 );
  display.print( currTemp );
  display.print( (char) 247 );
  display.print( "C" );
  display.setCursor( 25, 0 );
  display.setTextSize( 1 );
  display.print( "Press to upload" );
  display.display();
}


void refreshOffline()              /* 刷新测温页面 */
{
  currTemp = mlx.readObjectTempC() + 3.3;
  display.clearDisplay();
  display.setCursor( 25, 10 );
  display.setTextColor( WHITE );
  display.print( "OFFLINE" );
  display.setCursor( 25, 30 );
  display.setTextSize( 2 );
  display.print( currTemp );
  display.print( (char) 247 );
  display.print( "C" );
  display.display();
}


void refreshHomePage()          /* 刷新主页 */
{
  display.clearDisplay();
  display.setTextSize( 2 );
  display.setCursor( 0, 0 );
  for ( int i = 0; i < 4; i++ )
  {
    if ( i == choseP )
      display.setTextColor( BLACK, WHITE );
    else display.setTextColor( WHITE, BLACK );
    display.print( uname[i] );
    display.print( " " );
    display.print( temperature[i] );
    display.println( "C" );
  }
  display.display();
}


void ICACHE_RAM_ATTR upCallback()
{
  if ( atPage == true ) return;
  if(choseP > 0) choseP--;
  Serial.println("UP PRESSED");
  needRefreshHome = true;
}


void ICACHE_RAM_ATTR downCallback()
{
  if ( atPage == true ) return;
  if(choseP < 3) choseP++;
  Serial.println("DOWN PRESSED");
  needRefreshHome = true;
}


void ICACHE_RAM_ATTR midCallback()
{
  Serial.println("MID PRESSED");
  if ( atPage == false )
  {
    atPage = true;
    needRefreshTemp = true;
  }
  if ( atPage == true )
  {
    atPage = false;
    needRefreshHome = true;
    needSubmit = true;
  }
}


void setup()
{
  Serial.begin( 9600 );

  mlx.begin();
  Serial.println("Before Pullup");
  pinMode( UP, INPUT_PULLUP );
  pinMode( DOWN, INPUT_PULLUP );
  pinMode( MID, INPUT_PULLUP );
  Serial.println("Before Register");
  attachInterrupt( UP, upCallback, FALLING );
  attachInterrupt( DOWN, downCallback, FALLING );
  attachInterrupt( MID, midCallback, FALLING );
  Serial.println("After Register");

  WiFi.macAddress( MAC_array_STA );
  for ( int i = 0; i < sizeof(MAC_array_STA); ++i )
    sprintf( MAC_char_STA, "%s%02x:", MAC_char_STA, MAC_array_STA[i] );

  display.begin( SSD1306_SWITCHCAPVCC, 0x3C );
  display.clearDisplay();
  display.setTextColor( WHITE );
  display.setTextSize( 2 );
  display.setCursor( 28, 28 );
  display.print( "WELCOME" );
  display.setTextWrap( false );
  display.display();
  delay( 500 );
  display.setTextSize( 1 );

  WiFi.mode( WIFI_STA );
  WiFi.begin( "NKU_WLAN" );

  for ( uint8_t i = 0; WiFi.status() != WL_CONNECTED && i <= 15; i++ )
  {
    display.clearDisplay();
    display.setCursor( 0, 10 );
    display.print( ">Connecting NKU_WLAN<" );
    display.print( i );
    display.display();
    delay( 1000 );
  }

  display.clearDisplay();
  display.setCursor( 25, 10 );
  getConfig();
  if ( WiFi.status() != WL_CONNECTED )
    display.print( "NETWORK FAILED" );
  else display.print( "CONNECTED" );
  display.display();
  delay( 500 );
  refreshHomePage();
}


void loop()
{
  currTime = millis();

  if ( WiFi.status() != WL_CONNECTED && currTime - lastRefresh >= TEMP_REFRESH_FREQ )
  {
    refreshOffline();
    lastRefresh = currTime;
    return;
  }
  
  if (needRefreshHome){
    delay(50);
    if (digitalRead(UP) == LOW || digitalRead(DOWN) == LOW || digitalRead(MID) == LOW){
      refreshHomePage();
      needRefreshHome = false;
    }
  }
  if (needRefreshTemp){
    delay(50);
    if (digitalRead(MID) == LOW){
      Serial.println("Trying to refresh temp");
      refreshTemp();
      needRefreshTemp = false;
      Serial.println("Not a bussiness of refreshing temp");
    }
  }

  if (needSubmit) {
    delay(50);
    if (digitalRead(MID) == LOW){
      Serial.println("Trying to submit");
      //submitTemp();
      needSubmit = false;
      Serial.println("Not a bussiness of submiting temp");
    }
  }

  Serial.println(digitalRead( MID ) != LOW);
  Serial.println(atPage == true);
  Serial.println(currTime - lastRefresh >= TEMP_REFRESH_FREQ);
  if ( atPage == true && currTime - lastRefresh >= TEMP_REFRESH_FREQ )
  {
    Serial.println(currTime);
    refreshTemp();
    lastRefresh = currTime;
  }
}
