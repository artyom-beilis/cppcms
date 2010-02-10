#!/usr/bin/env python
# coding=utf-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
import sys
import socket
import time
import os.path
import traceback
import random

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

def test_io(name,method,input_f,output_f,seed=12):
    try:
        input_f=os.path.dirname(sys.argv[0]) + "/" + input_f;
        output_f=os.path.dirname(sys.argv[0]) + "/" + output_f;
        f=open(input_f,'rb')
        input=f.read()
        f.close()
        f=open(output_f,'rb')
        output=f.read();
        f.close()
        s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);
        global target
        s.connect(target)
        s.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
        result=''
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
    check(name,result,output)


def test_all(name):
    input_f = 'proto_test_' + name + '.in'
    output_f = 'proto_test_' + name + '.out'
    test_io(name+' normal','normal',input_f,output_f)
    test_io(name+' random 1','random',input_f,output_f,1)
    test_io(name+' random 2','random',input_f,output_f,15)
    test_io(name+' random 3','random',input_f,output_f,215)
    test_io(name+' shortest','shortest',input_f,output_f)


global target

def usege():
    print 'Usage proto_test.py (http|fastcgi_tcp|fastcgi_unix|scgi_tcp|scgi_unix)'
    sys.exit(1)

if len(sys.argv) != 2:
    usege()

test=sys.argv[1]

if test=='http' or test=='fastcgi_tcp' or test=='scgi_tcp':
    target=('localhost',8080)
else:
    target='/tmp/cppcms_test_socket'

if test=='http':
    test_all('http_1')
    test_all('http_2')
    test_all('http_3')
    test_all('http_4')
    test_all('http_5')
elif test=='fastcgi_tcp' or test=='fastcgi_unix':
    test_all('fastcgi_1')
elif test=='scgi_tcp' or test=='scgi_unix':
    test_all('scgi_1')
else:
    usege()
