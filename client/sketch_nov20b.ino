#include <Adafruit_MLX90614.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <FS.h>
//#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
 
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SERVER_IP "1.1.1.1"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
 
double temp_amb;
double temp_obj;
uint8_t MAC_array_STA[6];
char MAC_char_STA[18];

char* name[4]={"aaa","bbb","ccc","ddd"};
char* state[4]={"F","F","F","F"};
bool isChosen[4]={true,false,false,false}; 
float temperature[4]={0};

//文件系统函数
/*
void writeFile(const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  delay(2000); // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void deleteFile(const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (LittleFS.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}
*/

//网络相关函数
 /*
bool getConfig()
{
  char configUrl[100];
  WiFi.macAddress(MAC_array_STA);
  for (int i = 0; i < sizeof(MAC_array_STA); ++i) {
    sprintf(configUrl,"%s%02x:", MAC_char_STA, MAC_array_STA[i]);
  }
  Serial.println(configUrl);
  http.setTimeout(5000);
  http.begin(configUrl);
  int httpCode = http.GET();
  if(httpCode > 0 && httpCode == HTTP_CODE_OK)
  {
    deleteFile("config.txt");
    writeFile("config.txt",http.getString().c_str());
    return true;
  }
  else
    return false;
}
*/

bool submitTemp(String name, float temp){
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    http.begin(client, "http://" SERVER_IP "/submit_temp/"); //HTTP
    http.addHeader("Content-Type", "application/json");
    StaticJsonDocument<200> doc;
    doc['name'] = name;
    doc['temp'] = temp;
    char myDoc[measureJson(doc) + 1];
    serializeJson(doc, myDoc, measureJson(doc) + 1);
    Serial.println(myDoc);
    
    int httpCode = http.POST(myDoc);

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        http.end();
        return true;
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}

bool refreshData(){
  
}

//按钮相关
void buttonUp(int &i){
 isChosen[i]=false;
 i=(i+3)%4;
 isChosen[i]=true;
}
void buttonDown(int &i){
 isChosen[i]=false;
 i=(i+1)%4;
 isChosen[i]=true;
}
void buttonOK(){
  display.display();
  delay(1000);
  display.clearDisplay();
  display.print("START");
  delay(1000);
}
uint8_t getTouch()
{
  
}

//测温函数
float measure(){
  temp_amb = mlx.readAmbientTempC();
  temp_obj = mlx.readObjectTempC();
  for(int i=0;i<3;i++){   
    Serial.print("Room Temp = ");
    Serial.println(temp_amb);
    Serial.print("Object temp = ");
    Serial.println(temp_obj);
 
    display.clearDisplay();
    display.setCursor(25,10);  
    display.setTextColor(WHITE);
    display.setCursor(25,30);
    display.setTextSize(2);
    display.print(temp_obj);
    display.print((char)247);
    display.print("C");
    display.display();
    delay(100);
  }
  return temp_obj;//相关计算待做
}

void setup()
{
  Serial.begin(9600);
  mlx.begin();         //Initialize MLX90614
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(28,28);
  display.print("WELCOME");
  display.setTextWrap(false);
  display.display();
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin("NKU_WLAN");  
  delay(1000);
  uint8_t i=0;
  for(; WiFi.status() != WL_CONNECTED&&i<=15; i++)
  {
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setCursor(0,10);
      display.setTextSize(1);
      display.print(">Connecting NKU_WLAN<");
      display.print(i);
      display.display();
      delay(1000);
  }
    if(i > 15){
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setCursor(25,10);
      display.setTextSize(1);
      display.print("NETWORK FAILED");
      display.display();
      delay(1000);
    }
    else{
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setCursor(25,10);
      display.setTextSize(1);
      display.print("CONNECTED");
      display.display();
      delay(1000);
    }
}

void loop()
{
  static int i=0;

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
    
  for(int i=0;i<4;i++){
    if(isChosen[i]){
      display.setTextColor(BLACK,WHITE);
    }
    else display.setTextColor(WHITE,BLACK);
    display.print(name[i]);
    display.print(" ");
    display.print(temperature[i]);
    display.println("C");
  }
    
  buttonOK();

  temperature[i]=measure();
    
  delay(1000);

  state[i]="T";

  buttonDown(i);
}
