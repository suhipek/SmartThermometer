from flask import Flask, render_template, request, redirect
app = Flask(__name__)
from manage import *
import json

@app.route('/')
def student():
    return render_template('404.html')

@app.route('/manage',methods = ['POST', 'GET'])
def manage():
    if request.method == 'GET':
        device_name, profiles = get_profiles(request.args['mac'])
        return render_template("manage.html", \
            device_name = device_name, profiles = profiles, mac = request.args['mac'])

@app.route('/submit_temp',methods = ['POST', 'GET'])
def submit_temperature():
    if request.method == 'POST':
        try:
            print(request.json)
            print(submit_temp(request.json['mac'], request.json['name'], request.json['temp']))
            print(type(request.json['mac']))
            print(type(request.json['temp']))
            if submit_temp(request.json['mac'], request.json['name'], str(request.json['temp'])) == True:
                return 'success'
            else: return 'error'
        except OSError: return 'error'

@app.route('/change_name',methods = ['POST', 'GET'])
def change_name():
    if request.method == 'GET':
        return render_template("change_name.html", \
            mac=request.args['mac'], device_name=get_profiles(request.args['mac'])[0])
    if request.method == 'POST':
        try:
            if change_device_name(request.form['mac'], request.form['name']):
                return redirect("manage?mac={}".format(request.form['mac']))
            else: return '无法新增设备'
        except: return '失败'

@app.route('/add_device',methods = ['POST', 'GET'])
def new_device():
    if request.method == 'GET':
        return render_template("add_device.html")
    if request.method == 'POST':
        try:
            if create_device(request.form['mac']):
                return redirect("manage?mac={}".format(request.form['mac']))
            else: return '创建失败'
        except: return '创建失败'

@app.route('/add_user',methods = ['POST', 'GET'])
def new_user():
    if request.method == 'GET':
        return render_template("add_user.html", mac=request.args['mac'])
    if request.method == 'POST':
        try:
            print(request.form['mac'], request.form['name'])
            print('a')
            if add_user(request.form['mac'], request.form['name']):
                return redirect("manage?mac={}".format(request.form['mac']))
            else: return '错误'
        except: return '失败'

@app.route('/get_config',methods = ['POST', 'GET'])
def get_con():
    if request.method == 'POST':
        print(request.json)
        print(get_config(str(request.json['mac'])))
        return get_config(str(request.json['mac']))
    else: return 'error'

@app.route('/delete_user',methods = ['POST', 'GET'])
def deleteUser():
    if request.method == 'GET':
        if delete_user(request.args['mac'],request.args['user']):
            return redirect("manage?mac={}".format(request.args['mac']))
        else: return "Error"
    else: return 'error'

if __name__ == '__main__':
    app.run(debug=True)
