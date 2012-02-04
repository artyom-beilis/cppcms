#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


import httplib

class Cookie:

    def __init__(self,content=''):
        self.max_age = None
        self.domain = None
        self.path = None
        self.name = ''
        self.value = ''
        secure = False

        values=content.split(';')
        self.name = values[0].strip().split('=')[0]
        self.value = values[0].strip().split('=')[1]
        for vin in values[1:]:
            v=vin.strip();
            vals = v.split('=')
            parname = vals[0].lower()
            if parname=='domain':
                self.domain=vals[1]
            elif parname == 'path':
                self.path=vals[1]
            elif parname == 'max-age':
                self.max_age = vals[1]

    def __str__(self):
        s=self.pair()
        if self.domain:
            s+='; Domain=' + self.domain
        if self.path:
            s+='; Path='+self.path
        if self.max_age:
            s+='; Max-Age='+self.max_age
        return s

    def pair(self):
        return self.name + '=' + self.value 

def parse_cookies(response):
    cookies=[]
    hdr = response.getheader('set-cookie')
    if hdr:
        vals = hdr.split(',')
        for val in vals:
            cookies.append(Cookie(val))
    cm={}
    for c in cookies:
        cm[c.name]=c
    return cm

class Session:
    def __init__(self):
        self.state={}
        self.received={}

    def getcookies(self):
        allcookies=''
        for key,value in self.state.items():
            if allcookies == '':
                allcookies = value.pair()
            else:
                allcookies+= '; ' + value.pair()
        return allcookies

    def transmit(self,url):
        received={}
        conn=httplib.HTTPConnection('127.0.0.1',8080)
        conn.request('GET','/test' + url,None,{'Cookie':self.getcookies()})
        r=conn.getresponse()
        content = r.read();
        self.received=parse_cookies(r)
        self.update_state()
        return content
    
    def update_state(self):
        print "Got following cookies"
        for name,cookie in self.received.items():
            print cookie
            if name in self.state:
                if cookie.max_age=='0':
                    del self.state[name]
                else:
                    self.state[name] = cookie
            else:
                if cookie.max_age != '0':
                    self.state[name] = cookie
        print "---------------------"
        

