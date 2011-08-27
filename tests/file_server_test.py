#!/usr/bin/env python
# coding=utf-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
import socket
import time
import traceback
import random
import sys
import os


def test(x):
    if not x:
        print "Error"
        traceback.print_stack()
        sys.exit(1)

def make_sock():
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);
    s.connect(('localhost',8080));
    s.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
    return s;


def test_request(url,status,content='ignore',valid=[],notvalid=[]):
    print "-- Testing %s" % url
    s=make_sock();
    s.send('GET %s HTTP/1.0\r\n\r\n' % url);
    text = ''
    while 1:
        tmp = s.recv(1000);
        if len(tmp) == 0:
            break;
        text = text + tmp;
    exp = 'HTTP/1.0 ' + str(status) + ' '
    test(text[:len(exp)]==exp)
    parts = text.split('\r\n\r\n');
    real_content = ''
    if len(parts)>=2:
        real_content = parts[1]
    if content != 'ignore':
        test(real_content == content + '\n')
    for v in valid:
        test(real_content.find(v) >= 0)
    for v in notvalid:
        test(real_content.find(v) == -1)



do_listing = False 
if len(sys.argv) == 2:
    do_listing = sys.argv[1] == 'listing'



print "- Testing normal requests"

if not do_listing:
    test_request('/',404)
else:
    test_request('/',200,valid=['foo/','bar/','test.txt'],notvalid=['..','test.txt/','.svn'])
    
test_request('/test.txt',200,'/test.txt')
test_request('/foo/test.txt',200,'/foo/test.txt')
test_request('/bar/test.txt',200,'/bar/test.txt')
test_request('/bar/index.html',200,'/bar/index.html')
test_request('/bar/',200,'/bar/index.html')
test_request('/bar',302)
if not do_listing:
    test_request('/foo',404)
    test_request('/foo/',404)
else:
    test_request('/foo',302)
    test_request('/foo/',200,valid=['..','ooooooong_fiiiiii']);

test_request('/file+with+space.txt',200,'file with space')
test_request('/file%20with%20space.txt',200,'file with space')

print "- Testing alias"


test_request('/alias',302)
test_request('/alias/',200,'/al/index.html')
test_request('/alias/test.txt',200,'/al/test.txt')
test_request('/alias/foo/test.txt',200,'/al/foo/test.txt')

if os.name == 'posix':
    print "- Testing symlinks"
    test_request('/no.txt',404)
    test_request('/yes.txt',200,'/yes')

print "- Testing directory traversal"


test_request('/foo/../bar/test.txt',200,'/bar/test.txt')
test_request('/../al/test.txt','404')
test_request('/../never.txt','404')
test_request('/../www/test.txt','404')
test_request('/../wwwfile.txt','404')
test_request('/aliasfile.txt',404)
test_request('/../alias/never.txt','404')
test_request('/%2e%2e/never.txt','404')
test_request('/..%c0%afnever.txt','404')

