#!/usr/bin/env python
# coding=utf-8

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
from __future__ import print_function
try:
    import httplib
except:
    import http.client as httplib

import sys
import gzip

try:
    from StringIO import StringIO
except: # StringIO moved to io in python 3
    from io import BytesIO as StringIO

def test(name,A,B):
    if A != B:
        print("-----Actual--")
        print(A,"---Expected--")
        print(B,"-------------")
        traceback.print_tb()
        sys.exit(1)
    else:
        print("Ok:",name)


def decompress(content):
    virt = StringIO(content)
    zstream = gzip.GzipFile(mode='r', fileobj=virt)
    result = zstream.read()
    return result

def big():
    slist=[]
    for x in range(0,100000):
        slist.append(str(x)+'\n')
    return (''.join(slist)).encode()


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

test_valid('/ca',True,True,b'test a')
test_valid('/ca',False,False,b'test a')

test_valid('/cb',True,True,b'test b')
test_valid('/cb',False,False,b'test b')
bg=big()
test_valid('/cbig',True,True,bg)
test_valid('/cbig',False,False,bg)

test_valid('/bin',True,False,b'binary')
test_valid('/bin',False,False,b'binary')
test_valid('/not',True,False,b'not compressed')
test_valid('/not',False,False,b'not compressed')


