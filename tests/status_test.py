#!/usr/bin/env python
# coding=utf-8

import httplib
import sys

def test_status(h,stat):
    if h.status != stat:
        print "Status mistmatch:",h.status,"!=",stat
        sys.exit(1)

def test_valid(url,status):
    h=httplib.HTTPConnection('localhost:8080');
    h.request('GET','/test' + url)
    r=h.getresponse()
    if(r.status!=status):
        print "Failed", url , r.status,"!=",status
        sys.exit(1)
    else:
        print "Ok",url,status

test_valid('/normal',200)
test_valid('/throws',500)
test_valid('/invalid',404)

if len(sys.argv) == 2 and sys.argv[1]=='async':
    test_valid('/delayed',200)


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
