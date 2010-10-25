#!/usr/bin/env python
# coding=utf-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
import sys
import socket
import time
import os.path
import traceback
import random
import toscgi
import tofcgi

def load_file(file_name):
    file_name = os.path.dirname(sys.argv[0]) + "/" + file_name
    f=open(file_name,'rb')
    input=f.read()
    f.close()
    return input

def test_io(input,socket_type,target):
    try:
        s=socket.socket(socket_type,socket.SOCK_STREAM);
        s.connect(target)
        if socket_type==socket.AF_INET:
            s.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
        s.sendall(input)
        for x in xrange(0,100):
            chunk = s.recv(1024)
            if chunk == '':
                break
        s.close();
    except socket.error:
        pass

def usege():
    print './disco_test.py (http|fastcgi_tcp|scgi_tcp|fastcgi_unix|scgi_unix)'

test=sys.argv[1]

if test=='http' or test=='fastcgi_tcp' or test=='scgi_tcp':
    target=('localhost',8080)
    socket_type=socket.AF_INET
else:
    target=('/tmp/cppcms_test_socket')
    socket_type=socket.AF_UNIX

if test=='http':
    input = load_file('disco_test_norm.in');
    test_io(input,socket_type,target);
    input = load_file('disco_test_gzip.in');
    test_io(input,socket_type,target);
elif test=='fastcgi_tcp' or test=='fastcgi_unix':
    input = tofcgi.to_fcgi_request(load_file('disco_test_norm_cgi.in'));
    test_io(input,socket_type,target);
    input = tofcgi.to_fcgi_request(load_file('disco_test_gzip_cgi.in'));
    test_io(input,socket_type,target);
elif test=='scgi_tcp' or test=='scgi_unix':
    input = toscgi.toscgi(load_file('disco_test_norm_cgi.in'));
    test_io(input,socket_type,target);
    input = toscgi.toscgi(load_file('disco_test_gzip_cgi.in'));
    test_io(input,socket_type,target);
else:
    usege()
