#!/usr/bin/env python
# coding=utf-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

import httplib
import sys


def check(name,A,B):
    if A != B:
        print "Error :" + name 
        print "-----Actual--"
        print A,"---Expected--"
        print B,"-------------"
        f1=open('actual.txt','wb')
        f2=open('expected.txt','wb')
        f1.write(A)
        f2.write(B)
        f1.close()
        f2.close()
        sys.exit(1)
    else:
        print "Ok:"+name

class Nothing:
    pass

def test_valid(headers,expected):
    h=httplib.HTTPConnection('localhost:8080');
    h.request('GET','/test','',{ 'Cookie': headers } );
    r=h.getresponse()
    check(headers,r.read(),expected)

test_valid('test=foo','test:foo\n')
test_valid('test=','test:\n')
test_valid('test','test:\n')
test_valid('test="\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D"; z=x','test:\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D\nz:x\n');
test_valid('test="\xD7\xA9\\"\xD7\x9C\xD7\x95\xD7\x9D"; z=x','test:\xD7\xA9"\xD7\x9C\xD7\x95\xD7\x9D\nz:x\n');
test_valid('a=\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D; z=x','z:x\n');




