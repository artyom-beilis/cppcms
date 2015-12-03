#!/usr/bin/env python
# coding=utf-8
#
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
#
import httplib
import sys
import re
import time
import datetime
import socket

def test(x):
    if not x:
        raise RuntimeError("Failed")
def now():
    return datetime.datetime.now().strftime("%H:%M:%S.%f")

class Conn():
    num=re.compile('^[0-9]+$')
    def __init__(self,path):
        self.path = path
        print now(),'GET',path
        self.s=socket.socket(socket.AF_INET, socket.SOCK_STREAM);
        self.s.connect(('127.0.0.1',8080))
        self.s.send('GET ' + path + ' HTTP/1.0\r\n\r\n')
    def get(self):
        print now(),'READ ',self.path
        response = ''
        while True:
            tmp=self.s.recv(1000)
            if len(tmp) == 0:
                self.s.close()
                break
            response = response + tmp
        body = response.split('\r\n\r\n')[1]

        #r=self.h.getresponse()
        #body = r.read()
        r={}
        for s in body.split('\n'):
            if s=='':
                break
            ss=s.split('=')
            if Conn.num.match(ss[1]):
                r[ss[0]]=int(ss[1])
            else:
                r[ss[0]]=ss[1]
        print now(), "Got",r
        return r

def pool_many(url,cb=None):
    a=[None]*10
    for i in xrange(0,len(a)):
        a[i]=Conn(url)
    for i in xrange(0,len(a)):
        r=a[i].get()
        if cb:
                cb(r)
        a[i]=None

def test_sync():
    st=Conn('/test/stats?id=/sync').get()
    test(st["total"]==0)

    c1 = Conn('/sync?sleep=0.5') # T1
    time.sleep(0.1)
    c2 = Conn('/sync/work?sleep=1.0') # T2
    time.sleep(0.1)
    c3 = Conn('/sync/work?sleep=1.0') # T1
    time.sleep(0.1)
    r1 = c1.get()
    test(r1['original_thread_id'] == r1['thread_id'])
    test(r1['app_id']==1)
    c4 = Conn('/sync')
    r4 = c4.get()
    test(r4['app_id']==1)
    test(r4['thread_id']!=r4['original_thread_id'])
    r2 = c2.get()
    r3 = c3.get()

    pool_many('/sync?sleep=0.2')
    st=Conn('/test/stats?id=/sync').get()
    test(st["total"]==2)
    test(st["current"]==2)


test_sync()

def test_sync_prep():
    n='/sync/prepopulated'
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==2)

    c1 = Conn(n+'?sleep=0.2') 
    c2 = Conn(n+'?sleep=0.2') 
    r1=c1.get()
    r2=c2.get()
    test(r1["app_id"]!=r2["app_id"])
    test(r1["original_thread_id"]==1)
    test(r2["original_thread_id"]==1)
    test(r1["thread_id"]!=r2["thread_id"])
    test(r1["thread_id"] >= 1000)
    test(r2["thread_id"] >= 1000)

    pool_many(n+'?sleep=0.2')
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==2)
    test(st["current"]==2)

test_sync_prep()


def test_sync_ts():
    st=Conn('/test/stats?id=/sync/tss').get()
    test(st["total"]==0)

    c1 = Conn('/sync/tss?sleep=0.5') # T1
    time.sleep(0.1)
    c2 = Conn('/sync/work?sleep=1.0') # T2
    time.sleep(0.1)
    c3 = Conn('/sync/work?sleep=1.0') # T1
    time.sleep(0.1)
    r1 = c1.get()
    test(r1['original_thread_id'] == r1['thread_id'])
    test(r1['app_id']==1)
    c4 = Conn('/sync/tss')
    r4 = c4.get()
    test(r4['app_id']==2)
    test(r4['thread_id']==r4['original_thread_id'])
    r2 = c2.get()
    r3 = c3.get()

    def cb(r):
        test(r['thread_id']==r['original_thread_id'])

    pool_many('/sync/tss?sleep=0.2',cb)
    st=Conn('/test/stats?id=/sync/tss').get()
    test(st["total"]==2)
    test(st["current"]==2)

test_sync_ts()
