#encoding:utf-8
import json
import requests
import time
import datetime
import os
#import ast #字符转字典 user_dict = ast.literal_eval(user)
from urllib.parse import urlencode

#喵提醒
def pushplus_post(pushplus_token,name,res):
    url = 'http://miaotixing.com/trigger'
    msg = {
        "id": pushplus_token,
        "text": "亲爱的" + name + "：\n您的" + json.dumps(res, ensure_ascii=False) + "\n时间:" + str(
            datetime.datetime.now().replace(microsecond=0)),
        "type": "json"
    }
    requests.post(url, data=msg)
    print(name + res + '\n')

class WoZaiXiaoYuanPuncher:
    def __init__(self, item):
        # 账号数据
        self.data = item
        # 登陆接口
        self.loginUrl = "https://gw.wozaixiaoyuan.com/basicinfo/mobile/login/username"
        # 请求头
        self.header = {
            "Accept-Encoding": "gzip, deflate, br",
            "Connection": "keep-alive",
            "User-Agent": "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.138 Safari/537.36 MicroMessenger/7.0.9.501 NetType/WIFI MiniProgramEnv/Windows WindowsWechat",
            "Content-Type": "application/json;charset=UTF-8",
            "Content-Length": "2",
            "Host": "gw.wozaixiaoyuan.com",
            "Accept-Language": "en-us,en",
            "Accept": "application/json, text/plain, */*",
        }
        # 请求体（必须有）
        self.body = "{}"

    # 登陆
    def login(self):
        url = self.loginUrl + "?username=" + str(self.data['username']) + "&password=" + str(self.data['password'])
        print("----url-----")
        print(url)
        self.session = requests.session()
        print("----session-----")
        print(self.session)
        # 登陆
        response = self.session.post(url=url, data=self.body, headers=self.header)
        print("----resopnse-----")
        print(response)
        res = json.loads(response.text)
        print("----res-----")
        print(res)
        if res["code"] == 0:
            print("登陆成功")
            jwsession = response.headers['JWSESSION']
            self.setJwsession(jwsession)
            return True
        else:
            print("登陆失败，请检查账号信息")
            res = res["message"]
            pushplus_post(self.data['PushPlus_token'], self.data['name'], res)
            return False

    # 设置JWSESSION
    def setJwsession(self, jwsession):
        self.header['JWSESSION'] = jwsession

    # 执行打卡
    # 参数seq ： 当前打卡的序号
    def doPunchIn(self):
        self.header['Host'] = "student.wozaixiaoyuan.com"
        self.header['Content-Type'] = "application/x-www-form-urlencoded"
        self.header["Content-Length"] = "453"
        self.header['refer'] = 'https://servicewechat.com/wxce6d08f781975d91/175/page-frame.html'
        url = "https://student.wozaixiaoyuan.com/health/save.json"
        sign_data = {
        'answers': '',  
        'latitude': '34.3667',
        'longitude': '109.21421',
        'country': '中国',
        'city': 'xxx市',
        'district': 'xxx区',
        'province': 'xxx省',
        'township': 'xxx道',
        'street': 'xxxx街',
        'areacode': 'xxx'
        }
        #拼接家庭住址
        answer_str = '["0","1","1","1","0",'
        answer_str += '"' + self.data['home_addr'] + '"' + ']'
        #完善报头
        sign_data['answers'] = answer_str
        data = urlencode(sign_data)
        response = self.session.post(url=url, data=data, headers=self.header)
        response = json.loads(response.text)
        # 打卡情况
        if response["code"] == 0:
            res = "健康打卡成功,期待下一次成功"
            print(res)
        elif response["code"] == 1:
            res = "今日健康打卡已结束"
            print("今日健康打卡已结束") 
        else:
            print("健康打卡失败")
            res = "健康打卡失败,请自行手动打卡并联系管理员"
        pushplus_post(self.data['PushPlus_token'], self.data['name'], res)

    def SignCheckIn(self):
        self.header['Host'] = "student.wozaixiaoyuan.com"
        #self.header['refer'] = 'https://servicewechat.com/wxce6d08f781975d91/143/page-frame.html'
        self.header['refer'] = 'https://servicewechat.com/wxce6d08f781975d91/149/page-frame.html'
        self.header['content-type'] = 'application/x-www-form-urlencoded'
        listUrl = "https://student.wozaixiaoyuan.com/sign/getSignMessage.json"
        signUrl = "https://student.wozaixiaoyuan.com/sign/doSign.json"
        Spostdata = {
            "id": "",
            "signId": "",
            'latitude' : '34.3667',
            'longitude' : '109.21421',
            'country' : '中国',
            'city' : 'xxx市',
            'district' : 'xx区',
            'province' : 'xx省',
            'township' : 'xxx道',
            'street' : 'xxx街',
            'areacode' : 'xxxx'
        }
        #1. 获取签到ID
        data = {
            'page': '1',
            'size': '5'
        }
        data = urlencode(data)
        id_res = self.session.post(listUrl, headers=self.header, data=data).json()
        #失败直接返回
        if id_res and id_res['code'] == -10:
            id_res = "获取签到ID失败！请自行手动打卡并联系管理员!!"
            pushplus_post(self.data['PushPlus_token'], self.data['name'], res)
            return
        #成功继续签到
        else:
            Spostdata["id"] = id_res['data'][0]['logId']
            Spostdata["signId"] = id_res['data'][0]['id']

        self.header['content-type'] = "application/json"
        self.header.pop('Content-Length')
        res = self.session.post(signUrl, headers=self.header, data=json.dumps(Spostdata)).json()
        print(res)
        if (res['code'] == 0):
            res = "晚签到成功,期待下一次成功!"
            print(res)
        elif (res['code'] == 1):
            res = "晚签到失败,打卡时间已结束!"
            print(res)
        else:
            res = "晚签到失败，原因未知。请自行手动打卡并联系管理员！！！"
        pushplus_post(self.data['PushPlus_token'], self.data['name'], res)



def check(hour_now):
    path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    path = path + r"/config/"
    files = os.listdir(path)
    for file in files:
        with open(path + file,"r", encoding='utf-8') as f:
            info_dict = json.load(f)
            wzxy = WoZaiXiaoYuanPuncher(info_dict)
            loginStatus = wzxy.login()
            if(loginStatus == False):    
                continue
            # 健康打卡
            if (hour_now == 0) and (info_dict['health_check_self']):
                wzxy.doPunchIn()
            # 晚签到
            if (hour_now == 21) and (info_dict['sign_check_self']):
                wzxy.SignCheckIn()
        f.close()
    time.sleep(80)

if __name__ == "__main__":
    while True:
        current_time = datetime.datetime.now().replace(microsecond=0)
        current_hour = current_time.hour
        current_minutes = current_time.minute
        if current_hour == 21 and current_minutes == 1:
            check(current_hour)
            print("晚签到:%s", time.asctime())
        elif current_hour == 0 and current_minutes == 1:
            check(current_hour)
            print("健康打卡:%s",time.asctime())
        else:
            time.sleep(60)

