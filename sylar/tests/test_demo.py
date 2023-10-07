import requests







url = "http://192.168.209.129:8020/sylar/test1"
r = requests.get(url=url)
print(r.text)
print(r.status_code)
