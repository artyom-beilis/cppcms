#!/usr/bin/env python
# coding=utf-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
from __future__ import print_function
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
        print("Error :",name )
        print("-----Actual--")
        print(A,"---Expected--")
        print(B,"-------------")
        f1=open('actual.txt','wb')
        f2=open('expected.txt','wb')
        f1.write(A)
        f2.write(B)
        f1.close()
        f2.close()
        sys.exit(1)
    else:
        print("Ok:",name)

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

def get_socket():
    global socket_type
    s=socket.socket(socket_type,socket.SOCK_STREAM);
    global target
    s.connect(target)
    if socket_type==socket.AF_INET:
        s.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
    return s

def test_io(name,method,input_f,output_f,seed=12,load=load_file,parse=identity,time_limit=0.5):
    result=b''
    try:
        input=load(input_f)
        output=load_file(output_f)

        s=get_socket()

        if method=='normal':
            s.sendall(input)
            while 1:
                start = time.time()
                chunk = s.recv(1024)
                if chunk == b'':
                    break
                result=result+chunk
            end = time.time()
        elif method=='shortest':
            for k in range(len(input)):
                s.sendall(input[k:k+1])
                time.sleep(0.001)
            while 1:
                start = time.time()
                chunk = s.recv(1)
                if chunk == b'':
                    break;
                time.sleep(0.001)
                result=result+chunk
            end = time.time()
        elif method=='random':
            random.seed(seed);
            size = 0
            while size < len(input):
                chunk=input[size:size+random.randint(1,16)]
                size = size + len(chunk)
                s.sendall(chunk)
                time.sleep(0.001)
            while 1:
                start = time.time()
                chunk = s.recv(random.randint(1,16))
                if chunk == b'':
                    break;
                time.sleep(0.001)
                result=result+chunk
            end = time.time()
        else:
            print('Unknown method',method)
            sys.exit(1)
        if hasattr(socket,'SHUT_RDWR'):
            s.shutdown(socket.SHUT_RDWR)
        else:
            s.shutdown(2) 
        s.close()
        assert end - start < time_limit, ("Complete response withing %1.1f seconds, done in %1.1f" % (time_limit,end-start))
    except socket.error:
        pass
    check(name,parse(result),output)


def test_all(name,load=load_file,parse=identity,time_limit=0.5):
    input_f = 'proto_test_' + name + '.in'
    output_f = 'proto_test_' + name + '.out'
    test_io(name+' normal','normal',input_f,output_f,0,load,parse,time_limit)
    test_io(name+' random 1','random',input_f,output_f,1,load,parse,time_limit)
    test_io(name+' random 2','random',input_f,output_f,15,load,parse,time_limit)
    test_io(name+' random 3','random',input_f,output_f,215,load,parse,time_limit)
    test_io(name+' shortest','shortest',input_f,output_f,0,load,parse,time_limit)

def test_normal(name,load=load_file,parse=identity):
    input_f = 'proto_test_' + name + '.in'
    output_f = 'proto_test_' + name + '.out'
    test_io(name+' normal','normal',input_f,output_f,0,load,parse)

def test_scgi(name):
    def load(name):
        file=load_file(name)
        return toscgi.toscgi(file)
    test_all(name,load)

def test_scgi_normal(name):
    def load(name):
        file=load_file(name)
        return toscgi.toscgi(file)
    test_normal(name,load)

def transfer_all(s,inp):
    s.setblocking(1)
    s.sendall(inp)
    s.setblocking(0)
    result = b''
    slept=0
    while 1:
        try:
            tmp = s.recv(1024)
            if tmp == b'':
                raise Exception('Unexpected end of file')
            result+=tmp
            slept=0
        except socket.error:
            if slept:
                return result
            else:
                time.sleep(0.1)
                slept=1

def get_next_chunk(data):
    h_size = data.find(b'\r\n')
    size = int(data[0:h_size],16)
    chunk_start = h_size + 2
    chunk_end = chunk_start + size
    block_end = chunk_end+2;
    assert data[chunk_end:block_end] == b'\r\n'
    chunk = data[chunk_start:chunk_end]
    reminder = data[block_end:]
    return chunk,reminder

def from_http(data):
    if data == b'':
        return data
    split_point = data.find(b'\r\n\r\n')
    headers = data[:split_point+4]
    rest = data[split_point+4:]
    content_length = -1
    chunked = False;
    headers_out = []
    for header in headers.split(b'\r\n'):
        if header.find(b'Content-Length') == 0:
            content_length = int(header.split(b':')[1])
        elif header.find(b'Transfer-Encoding: chunked') == 0:
            chunked = True
        else:
            headers_out.append(header)
    headers = b'\r\n'.join(headers_out) 
    if content_length != -1:
        body = rest[:content_length]
        reminder = rest[content_length:]
        r1 = headers + body
        return r1 + from_http(reminder)
    elif chunked:
        body = b''
        while True:
            chunk,rest = get_next_chunk(rest)
            if chunk == b'':
                return headers + body + from_http(rest)
            body = body + chunk
    else:
        return headers + rest

    
    


def test_fcgi_keep_alive(name):
    input_f = 'proto_test_' + name + '.in'
    output_f = 'proto_test_' + name + '.out'
    inp = tofcgi.to_fcgi_request(load_file(input_f),tofcgi.TEST_KEEP_ALIVE)
    out = load_file(output_f)
    s=get_socket()
    res = tofcgi.from_fcgi_response(transfer_all(s,inp),tofcgi.TEST_KEEP_ALIVE)
    check('fcgi keep alive first ' + name,res,out)
    res = tofcgi.from_fcgi_response(transfer_all(s,inp),tofcgi.TEST_KEEP_ALIVE)
    check('fcgi keep alive second ' + name,res,out)
    if hasattr(socket,'SHUT_RDWR'):
        s.shutdown(socket.SHUT_RDWR)
    else:
        s.shutdown(2) 
    s.close()




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


def test_http(name,time_limit=0.5):
    test_all(name,parse=from_http,time_limit=time_limit)

global target
global socket_type

def usege():
    print('Usage proto_test.py (http|fastcgi_tcp|fastcgi_unix|scgi_tcp|scgi_unix)')
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
    print("Testing HTTP/1.0")
    test_http('http_1')
    test_http('http_2')
    test_http('http_3')
    test_http('http_4')
    test_http('http_5')
    print("Testing HTTP/1.1")
    test_http('http_6')
    test_http('http_7',time_limit=8.0)
    test_http('http_8')
elif test=='fastcgi_tcp' or test=='fastcgi_unix':
    test_fcgi_keep_alive('scgi_1')
    test_fcgi_keep_alive('scgi_2')
    test_fcgi_keep_alive('scgi_3')
    test_fcgi('scgi_1')
    test_fcgi('scgi_2')
    test_fcgi('scgi_3')
    print("Testing big pairs")
    test_fcgi_keep_alive('scgi_4')
    test_fcgi_keep_alive('scgi_5')
    test_fcgi('scgi_4')
    test_fcgi('scgi_5')
    print("Testing chunked pairs")
    test_fcgi('scgi_4',tofcgi.TEST_RANDOM)
    test_fcgi('scgi_5',tofcgi.TEST_RANDOM)
    print("Testing GET_VALUES")
    test_fcgi('scgi_1',tofcgi.TEST_GET_VALUES)
    print("Testing long message")
    test_fcgi('scgi_6',tofcgi.TEST_STANDARD)
elif test=='scgi_tcp' or test=='scgi_unix':
    test_scgi('scgi_1')
    test_scgi('scgi_2')
    test_scgi('scgi_3')
    print("Testing long message")
    test_scgi_normal('scgi_6')
else:
    usege()
