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
    return "    " + datetime.datetime.now().strftime("%H:%M:%S.%f")

class Conn:
    num=re.compile('^[0-9]+$')
    def __init__(self,path):
        self.path = path
        print now(),'GET',path
        self.s=socket.socket(socket.AF_INET, socket.SOCK_STREAM);
        self.s.connect(('127.0.0.1',8080))
        self.s.send('GET ' + path + ' HTTP/1.0\r\n\r\n')
    def get(self,exp404=False):
        print now(),'READ ',self.path
        response = ''
        while True:
            tmp=self.s.recv(1000)
            if len(tmp) == 0:
                self.s.close()
                break
            response = response + tmp
        r2 = response.split('\r\n\r\n')
        headers=r2[0]
        body = r2[1]
        first_header = headers.split('\r\n')[0]
        if exp404:
            print now(), "Got",first_header
            test(headers.find('HTTP/1.0 404')==0)
            return {'status' : 404 }

        r={}
        for s in body.split('\n'):
            if s=='':
                break
            ss=s.split('=')
            if Conn.num.match(ss[1]):
                r[ss[0]]=int(ss[1])
            else:
                r[ss[0]]=ss[1]
        print now(), "Got",first_header,r
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
    n='/sync'
    print n
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

    st=Conn('/test/unmount?id='+n).get()
    Conn(n).get(exp404 = True)
    time.sleep(0.1)
    test(Conn('/test/stats?id='+n).get()["current"]==0)


def test_sync_prep():
    n='/sync/prepopulated'
    print n
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

    Conn('/test/unmount?id='+n).get()
    Conn(n).get(exp404 = True)
    time.sleep(0.1)
    test(Conn('/test/stats?id='+n).get()["current"]==0)


def test_sync_ts():
    n='/sync/tss'
    print '/sync/tss'
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

    st=Conn('/test/unmount?id='+n).get()
    Conn(n).get(exp404 = True)
    time.sleep(0.1)
    test(Conn('/test/stats?id='+n).get()["current"]==2)


def test_sync_legacy():
    n='/sync/legacy'
    print n
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==0)

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

def test_async():
    n='/async'
    print n
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==0)

    c1 = Conn(n+'?sleep=0.2') 
    c2 = Conn(n+'?sleep=0.2') 
    r1=c1.get()
    r2=c2.get()
    test(r1["app_id"]==r2["app_id"])
    test(r1["original_thread_id"]==1)
    test(r1["thread_id"] == 1)
    test(r2["original_thread_id"]==1)
    test(r2["thread_id"] == 1)

    pool_many(n+'?sleep=0.0')
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)
    test(st["current"]==1)
    
    st=Conn('/test/unmount?id='+n).get()
    Conn(n).get(exp404 = True)
    time.sleep(0.1)
    test(Conn('/test/stats?id='+n).get()["current"]==0)

def test_async_prep():
    n='/async/prepopulated'
    print n
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)

    c1 = Conn(n+'?sleep=0.2') 
    r1=c1.get()
    test(r1["app_id"]==1)
    test(r1["original_thread_id"]==1)
    test(r1["thread_id"] == 1)

    pool_many(n+'?sleep=0.1')
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)
    test(st["current"]==1)

    st=Conn('/test/unmount?id='+n).get()
    Conn(n).get(exp404 = True)
    time.sleep(0.1)
    test(Conn('/test/stats?id='+n).get()["current"]==0)

def test_async_legacy():
    n='/async/legacy'
    print n
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)

    c1 = Conn(n+'?sleep=0.2') 
    r1=c1.get()
    test(r1["app_id"]==1)
    test(r1["original_thread_id"]==1)
    test(r1["thread_id"] == 1)

    pool_many(n+'?sleep=0.1')
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)
    test(st["current"]==1)

def test_async_temporary():
    n='/async/temporary'
    print n
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==0)
    
    Conn(n).get(exp404 = True)
    st=Conn('/test/install').get()
    test(st["install"]==1)

    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)
    test(st["current"]==1)
    c1 = Conn(n) 
    r1=c1.get()
    test(r1["app_id"]==1)
    test(r1["original_thread_id"]==1)
    test(r1["thread_id"] == 1)
    
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)
    test(st["current"]==1)
    
    st=Conn('/test/uninstall').get()
    test(st["install"]==0)
    
    time.sleep(0.1)
    st=Conn('/test/stats?id=' + n).get()
    test(st["total"]==1)
    test(st["current"]==0)

    Conn(n).get(exp404 = True)
    
def test_send():

    print "/sync/sender"

    r=Conn('/sync/sender?to=async&to_app=1').get()
    test(r["path"]=="/app")
    test(r["async"]==1)
    test(r["src_async"]==0)
    test(r["src_created"]==1)
    test(r["src_to_app"]==1)
    
    r=Conn('/sync/sender?to=async&to_app=1').get()
    test(r["path"]=="/app")
    test(r["async"]==1)
    test(r["src_async"]==0)
    test(r["src_created"]==0)
    test(r["src_to_app"]==1)

    r=Conn('/sync/sender?to=async').get()
    test(r["path"]=="/pool")
    test(r["async"]==1)
    test(r["src_async"]==0)
    test(r["src_created"]==0)
    test(r["src_to_app"]==0)
    
    r=Conn('/sync/sender?to=sync').get()
    test(r["path"]=="/pool")
    test(r["async"]==0)
    test(r["src_async"]==0)

    print "/async/sender"
    
    r=Conn('/async/sender?to=async&to_app=1').get()
    test(r["path"]=="/app")
    test(r["async"]==1)
    test(r["src_async"]==1)
    test(r["src_to_app"]==1)
    
    r=Conn('/async/sender?to=async').get()
    test(r["path"]=="/pool")
    test(r["async"]==1)
    test(r["src_async"]==1)
    
    r=Conn('/async/sender?to=sync').get()
    test(r["path"]=="/pool")
    test(r["async"]==0)
    test(r["src_async"]==1)
    

test_sync()
test_sync_prep()
test_sync_ts()
test_sync_legacy()
test_async()
test_async_prep()
test_async_legacy()
test_async_temporary()
test_send()
print "OK"
