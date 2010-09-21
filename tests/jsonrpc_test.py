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

def test_valid(name,params,ans,method='POST'):
    h=httplib.HTTPConnection('localhost:8080');
    headers = {"Content-type": "application/json"}
    h.request(method,'/test',params,headers)
    r=h.getresponse()
    test(name,r.read(),ans)

test_valid('sum','{"method" : "sum" , "params" : [ 1, 2 ], "id" : 1}','{"id":1,"error":null,"result":3}')
test_valid('sum wrong','{"method" : "sum" , "params" : [ 1, 10 ], "id" : null}','')
test_valid('sum wrong 1','{"method" : "sum" , "params" : [ 1 ], "id" : 1}',\
        '{"id":1,"error":"Invalid parametres number","result":null}')
test_valid('sum wrong 2','{"method" : "sum" , "params" : [ 1 ,"x" ], "id" : 1}',\
        '{"id":1,"error":"Invalid parameters","result":null}')
test_valid('div1','{"method" : "div" , "params" : [ 5, 2 ], "id" : "x"}','{"id":"x","error":null,"result":2}')
test_valid('div2','{"method" : "div" , "params" : [ 5, 0 ], "id" : 0}','{"id":0,"error":"Division by zero","result":null}')
test_valid('notify1','{"method" : "notify" , "params" : [ "notify" ], "id" : null}','')
test_valid('not notify','{"method" : "notify" , "params" : [ "notcalled" ], "id" : "x"}',\
            '{"id":"x","error":"The request should be notification","result":null}')

test_valid('both','{"method" : "both" , "params" : [ "notification" ], "id" : null}','')
test_valid('both','{"method" : "both" , "params" : [ "foo" ], "id" : 1}','{"id":1,"error":null,"result":"call:foo"}')

test_valid('smd','','{}','GET')
