#!/usr/bin/env python
# coding=utf-8

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
import httplib
import sys

def test(name,A,B):
    if A != B:
        print "Error :" + name 
        print "-----Actual--"
        print A,"---Expected--"
        print B,"-------------"
        sys.exit(1)
    else:
        print "Ok:"+name

def test_status(h,stat):
    if h.status != stat:
        print "Status mistmatch:",h.status,"!=",stat
        sys.exit(1)

def test_valid(name,url,params,ans,status):
    def get():
        h=httplib.HTTPConnection('localhost:8080');
        h.request('GET','/test' + url + '?' + params)
        r=h.getresponse()
        test_status(r,status)
        test(name+' GET',r.read(),ans)
    def post():
        h=httplib.HTTPConnection('localhost:8080');
        headers = {"Content-type": "application/x-www-form-urlencoded"}
        h.request('POST','/test' + url,params,headers)
        r=h.getresponse()
        test_status(r,status)
        test(name+' POST',r.read(),ans)
    if params=='':
        get()
    else:
        post()

test_valid('test1','','','path=\nmethod=GET\n',200)
test_valid('test2','/chunks','','path=/chunks\nmethod=GET\n',200)
test_valid('test3','/chunks','x=1&y=2','path=/chunks\nmethod=POST\nx=1\ny=2\n',200)
test_valid('status1','/status','','path=/status\nmethod=GET\n',201)
test_valid('status2','/status','x=1&y=2','path=/status\nmethod=POST\nx=1\ny=2\n',201)


