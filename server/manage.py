import pinyin
import time
import json

def get_profiles(mac: str):
    mac = mac.upper()
    try:
        with open('data/{}.csv'.format(mac)) as f:
            flines = f.readlines()
            device_name = flines[0].strip('\n')
            profiles = []
            for i in flines[1:]:
                line = i.strip('\n').split(',')
                line[1] = line[1].split(';')
                try:
                    line[2] = time.strftime("%m-%d %H:%M", \
                        time.localtime(float(line[2])))
                except: line[2] = "从来没测量过"
                profiles.append(line)
            return device_name, profiles
    except FileNotFoundError:
        return "未找到设备", []

def add_user(mac: str, name: str):
    mac = mac.upper()
    try:
        with open('data/{}.csv'.format(mac)) as f:
            data = f.readlines()
            f.close()
            if data[-1] == '\n': data.remove('\n')
            else: data[-1] += '\n' 
            if len(data) <= 5:
                data.append('{},36.50;36.50;36.50,-1,未测'.format(name))
            else: return False
        with open('data/{}.csv'.format(mac), mode='w') as f:
            print(data)
            f.writelines(data)
            f.close()
        return True
    except: return False

def change_device_name(mac: str, new_name: str):
    mac = mac.upper()
    try:
        with open('data/{}.csv'.format(mac)) as f:
            data = f.readlines()
            f.close()
        with open('data/{}.csv'.format(mac), mode='w') as f:
            f.writelines([new_name + '\n'] + data[1:])
            f.close()
        return True
    except: return False

def get_name_abbr(name: str):
    return pinyin.get_initial(name, delimiter="").upper()

def create_device(mac: str, device: str = '体温枪{}'):
    mac = mac.upper()
    try:
        with open('data/{}.csv'.format(mac), mode='r') as f:
            is_empty = f.readlines()
            f.close()
            if is_empty == []: raise FileNotFoundError
            else: return False
    except FileNotFoundError:
        try:
            with open('data/{}.csv'.format(mac), mode='w') as f:
                f.write(device.format(mac[-4:]))
                f.close()
                return True
        except FileNotFoundError: return False


def get_config(mac: str):
    mac = mac.upper()
    rd = {'name': [], 'status': [], 'temp': []}
    try:
        with open('data/{}.csv'.format(mac)) as f:
            data = f.readlines()
            f.close()
        for line in data[1:]:
            name = get_name_abbr(line.split(',')[0])
            print(line.split(','))
            temp = line.split(',')[1].split(';')[-1]
            stat = line.split(',')[3].strip('\n')
            rd['name'].append(name)
            rd['status'].append(stat)
            rd['temp'].append(temp)
        return json.dumps(rd)
    except: return "error"
        

def submit_temp(mac: str, user: str, temp: str):
    mac = mac.upper()
    try:
        with open('data/{}.csv'.format(mac)) as f:
            flines = f.readlines()
            f.close()
        flines[1:] = list(map(lambda x: x.split(','), flines[1:]))
        for line in flines[1:]:
            if get_name_abbr(line[0]) == user:
                line[1] = ';'.join(line[1].split(';')[1:] + [temp])
                line[2] = str(time.time())
                if float(temp) <= 37.5: line[3] = '正常'
                else: line[3] = '异常'
        flines[1:] = map(','.join, flines[1:])
        with open('data/{}.csv'.format(mac), mode='w') as f:
            f.writelines(flines)
        return True
    except: return False

