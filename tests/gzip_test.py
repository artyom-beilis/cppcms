#!/usr/bin/env python
# coding=utf-8

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

import httplib
import sys
import gzip
import StringIO

def test(name,A,B):
    if A != B:
        print "Error :" + name 
        print "-----Actual--"
        print A,"---Expected--"
        print B,"-------------"
        sys.exit(1)
    else:
        print "Ok:"+name

def decompress(content):
    virt = StringIO.StringIO(content)
    zstream = gzip.GzipFile(mode='r', fileobj=virt)
    result = zstream.read()
    return result
    


def test_valid(url,accepts,compressed,expected):
    h=httplib.HTTPConnection('localhost:8080');
    headers = {}
    if accepts:
        headers = { 'Accept-Encoding' : 'gzip'  }
    h.request('GET','/test' + url,'',headers)
    r=h.getresponse().read()
    if compressed:
        r=decompress(r)
    test(url,r,expected)

test_valid('/ca',True,True,'test a')
test_valid('/ca',False,False,'test a')
test_valid('/cb',True,True,'test b')
test_valid('/cb',False,False,'test b')
test_valid('/bin',True,False,'binary')
test_valid('/bin',False,False,'binary')
test_valid('/not',True,False,'not compressed')
test_valid('/not',False,False,'not compressed')


