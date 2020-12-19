import os
import time

def check_all():
    try:
        for i in os.listdir('data'):
            with open('data/{}'.format(i)) as f:
                data = f.readlines()
                f.close()
            for line in data[1:]:
                info = line.strip('\n').split(',')
                if time.time() - float(info[2]) >= 86400:
                    info[3] = '未测'
                info[-1] += '\n'
                line = ','.join(info)
            with open('data/{}'.format(i), mode='w') as f:
                f.writelines(data)
                f.close()
        print ('{}定时任务执行'.format(time.time()))
    except TypeError: print('{}定时任务异常'.format(time.time()))
        

