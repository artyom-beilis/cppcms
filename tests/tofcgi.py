#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
from __future__ import print_function
import sys
import random

BEGIN_REQUEST       = 1
ABORT_REQUEST       = 2
END_REQUEST         = 3
PARAMS              = 4
STDIN               = 5
STDOUT              = 6
STDERR              = 7
DATA                = 8
GET_VALUES          = 9
GET_VALUES_RESULT   = 10
UNKNOWN_TYPE        = 11

TEST_RANDOM = 1
TEST_GET_VALUES = 2
TEST_KEEP_ALIVE = 4
TEST_STANDARD = 8

def _chr(x):
    return b'%c' % x

def _ordp2(x):
    return ord(x)

def _ordp3(x):
    return x

_ord = _ordp2 if sys.version_info[0] == 2 else _ordp3

def escape(s):
    tmp=b''
    for k in range(len(s)):
        c=s[k:k+1]
        if 0 <= _ord(c) and _ord(c) <= 31:
            tmp+=b'\\%d' % _ord(c)
        else:
            tmp+=c
    return tmp

def to_body(text):
    return text[text.find(b'\n\n')+2:]

def make_pairs(pairs):
    def format_len(l):
        if l < 128:
            return _chr(l)
        else:
            res = _chr(128 + (l >> 24))
            res +=_chr((l >> 16) & 0xFF)
            res +=_chr((l>>8) & 0xFF)
            res +=_chr(l & 0xFF)
            return res
    res=b''
    for [key,value] in pairs:
        res+=format_len(len(key))+format_len(len(value))+key+value
    return res

def fcgi_rec_ord(type,content,request_id = 1):
    res=_chr(1)
    res+=_chr(type)
    res+=_chr(request_id >> 8) + _chr(request_id & 0xFF)
    cl = len(content)
    res+=_chr(cl >> 8) + _chr(cl & 0xFF)
    pl = (8 - cl % 8) % 8
    res+=_chr(pl)+b'\0'
    res+=content
    res+=b'\0'*pl
    return res

def get_headers(text):
    def get_basic_headers(text):
        res = []
        hdr = text[:text.find(b'\n\n')]
        for element in hdr.split(b'\n'):
            [key,value]=element.split(b':')
            res.append((key.upper().replace(b'-',b'_'),value))
        return res
    h=get_basic_headers(text)
    b=to_body(text)
    return [(b'CONTENT_LENGTH',str(len(b)).encode())] + h

def begin_request(keep_alive = 0):
    # responder 1 , keep_alive + padding
    content = b'\0\1'+_chr(keep_alive)+b'\0' * 5
    return fcgi_rec_ord(BEGIN_REQUEST,content)

def to_fcgi_request(text,flags = 0):
    request = b'';
    if flags & TEST_GET_VALUES:
        request+=fcgi_rec_ord(GET_VALUES,make_pairs([(b'FCGI_MPXS_CONNS',b''),(b'FCGI_MAX_CONNS',b''),(b'FCGI_MAX_REQS',b'')]),0)
    if flags & TEST_KEEP_ALIVE:
        request+=begin_request(1)
    else:
        request+=begin_request()
    headers = make_pairs(get_headers(text))
    if flags & TEST_RANDOM:
        while headers:
            l=random.randint(1,20)
            request+=fcgi_rec_ord(PARAMS,headers[:l])
            headers = headers[l:]
    else:
        request+=fcgi_rec_ord(PARAMS,headers)
    request+=fcgi_rec_ord(PARAMS,b'')
    body = to_body(text);
    if body:
        request+=fcgi_rec_ord(STDIN,body)
    request+=fcgi_rec_ord(STDIN,b'')
    return request

def from_fcgi_response(text,flags = 0):
    result = b''
    state = 0
    if flags & TEST_GET_VALUES:
        pairs = make_pairs([(b'FCGI_MPXS_CONNS',b'0'),(b'FCGI_MAX_CONNS',b'5'),(b'FCGI_MAX_REQS',b'5')])
        tmp = fcgi_rec_ord(GET_VALUES_RESULT,pairs,0)
        if text[:len(tmp)] != tmp:
            print('Invalid get values result \n[%s]\n[%s]\n' % (escape(tmp) , escape(text[:len(tmp)])))
            sys.exit(1)
        text=text[len(tmp):]
    while text:
        header = text[:8]
        if len(header)!=8 :
            print('Invalid header len')
            sys.exit(1)
        text = text[8:]
        type=_ord(header[1])
        if header[0:1]!=b'\1' or header[2:4]!=b'\0\1' or header[7:8]!=b'\0':
            tmp=escape(header)
            print('Invalid header [',tmp, ']')
            sys.exit(1)
        clength = (_ord(header[4]) << 8) + _ord(header[5])
        plength = _ord(header[6])
        if len(text) < clength + plength : 
            print('Unexpected EOF')
            sys.exit(1)
        if (clength + plength) % 8 != 0:
            print('Incorrect padding L=%d p=%d' % (clength,plength))
            sys.exit(1)
        content = text[:clength]
        text=text[clength + plength:]
        if state == 0:
            if type != STDOUT:
                print('Unexpected type %d' % type)
                sys.exit(1)
            if clength == 0:
                state = 1
            else:
                result +=content
        else:
            if type != END_REQUEST:
                print('Unexpected type %d' % type)
                sys.exit(1)
            if content != b'\0'*8:
                print('Unexpected End Request')
                sys.exit(1)
            if text:
                print('Unexpected input')
            return result


