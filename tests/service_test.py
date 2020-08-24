#!/usr/bin/env python
# coding=utf-8

try:
    import httplib
except:
    import http.client as httplib
import sys

h=httplib.HTTPConnection('127.0.0.1:8080');
h.request('GET','/test')
r=h.getresponse()
sys.stdout.write(r.read().decode())

