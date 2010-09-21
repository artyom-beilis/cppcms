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

def identity(x):
    return x

def load_file(file_name):
    file_name = os.path.dirname(sys.argv[0]) + "/" + file_name
    f=open(file_name,'rb')
    input=f.read()
    f.close()
    return input

def test_io(name,method,input_f,output_f,seed=12,load=load_file,parse=identity):
    result=''
    try:
        input=load(input_f)
        output=load_file(output_f)
        global socket_type
        s=socket.socket(socket_type,socket.SOCK_STREAM);
        global target
        s.connect(target)
        if socket_type==socket.AF_INET:
            s.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
        if method=='normal':
            s.sendall(input)
            while 1:
                chunk = s.recv(1024)
                if chunk == '':
                    break
                result=result+chunk
        elif method=='shortest':
            for char in input:
                s.sendall(char)
                time.sleep(0.001)
            while 1:
                chunk = s.recv(1)
                if chunk == '':
                    break;
                time.sleep(0.001)
                result=result+chunk
        elif method=='random':
            random.seed(seed);
            size = 0
            while size < len(input):
                chunk=input[size:size+random.randint(1,16)]
                size = size + len(chunk)
                s.sendall(chunk)
                time.sleep(0.001)
            while 1:
                chunk = s.recv(random.randint(1,16))
                if chunk == '':
                    break;
                time.sleep(0.001)
                result=result+chunk
        else:
            print 'Unknown method',method
            sys.exit(1)
        s.shutdown(socket.SHUT_RDWR)
        s.close()
    except socket.error:
        pass
    check(name,parse(result),output)


def test_all(name,load=load_file,parse=identity):
    input_f = 'proto_test_' + name + '.in'
    output_f = 'proto_test_' + name + '.out'
    test_io(name+' normal','normal',input_f,output_f,0,load,parse)
    test_io(name+' random 1','random',input_f,output_f,1,load,parse)
    test_io(name+' random 2','random',input_f,output_f,15,load,parse)
    test_io(name+' random 3','random',input_f,output_f,215,load,parse)
    test_io(name+' shortest','shortest',input_f,output_f,0,load,parse)

def test_normal(name,load=load_file,parse=identity):
    input_f = 'proto_test_' + name + '.in'
    output_f = 'proto_test_' + name + '.out'
    test_io(name+' normal','normal',input_f,output_f,0,load,parse)

def test_scgi(name):
    def load(name):
        file=load_file(name)
        return toscgi.toscgi(file)
    test_all(name,load)

def test_fcgi(name,flags = 0):
    def load(name):
        file=load_file(name)
        return tofcgi.to_fcgi_request(file,flags)
    def parse(str):
        return tofcgi.from_fcgi_response(str,flags)
    if flags == 0:
        test_all(name,load,parse)
    else:
        test_normal(name,load,parse)


global target
global socket_type

def usege():
    print 'Usage proto_test.py (http|fastcgi_tcp|fastcgi_unix|scgi_tcp|scgi_unix)'
    sys.exit(1)

if len(sys.argv) != 2:
    usege()

test=sys.argv[1]



if test=='http' or test=='fastcgi_tcp' or test=='scgi_tcp':
    target=('localhost',8080)
    socket_type=socket.AF_INET
else:
    target=('/tmp/cppcms_test_socket')
    socket_type=socket.AF_UNIX

if test=='http':
    test_all('http_1')
    test_all('http_2')
    test_all('http_3')
    test_all('http_4')
    test_all('http_5')
elif test=='fastcgi_tcp' or test=='fastcgi_unix':
    test_fcgi('scgi_1')
    test_fcgi('scgi_2')
    test_fcgi('scgi_3')
    print "Testing big pairs"
    test_fcgi('scgi_4')
    test_fcgi('scgi_5')
    print "Testing chunked pairs"
    test_fcgi('scgi_4',tofcgi.TEST_RANDOM)
    test_fcgi('scgi_5',tofcgi.TEST_RANDOM)
    print "Testing GET_VALUES"
    test_fcgi('scgi_1',tofcgi.TEST_GET_VALUES)
elif test=='scgi_tcp' or test=='scgi_unix':
    test_scgi('scgi_1')
    test_scgi('scgi_2')
    test_scgi('scgi_3')
else:
    usege()
