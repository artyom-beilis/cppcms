#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

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

def escape(s):
    tmp=''
    for c in s:
        if 0 <= ord(c) and ord(c) <= 31:
            tmp+='\\%d' % ord(c)
        else:
            tmp+=c
    return tmp

def to_body(text):
    return text[text.find('\n\n')+2:]

def make_pairs(pairs):
    def format_len(l):
        if l < 128:
            return chr(l)
        else:
            res = chr(128 + (l >> 24))
            res +=chr((l >> 16) & 0xFF)
            res +=chr((l>>8) & 0xFF)
            res +=chr(l & 0xFF)
            return res
    res=''
    for [key,value] in pairs:
        res+=format_len(len(key))+format_len(len(value))+key+value
    return res

def fcgi_record(type,content,request_id = 1):
    res=chr(1)
    res+=chr(type)
    res+=chr(request_id >> 8) + chr(request_id & 0xFF)
    cl = len(content)
    res+=chr(cl >> 8) + chr(cl & 0xFF)
    pl = (8 - cl % 8) % 8
    res+=chr(pl)+'\0'
    res+=content
    res+='\0'*pl
    return res

def get_headers(text):
    def get_basic_headers(text):
        res = []
        hdr = text[:text.find('\n\n')]
        for element in hdr.split('\n'):
            [key,value]=element.split(':')
            res.append((key.upper().replace('-','_'),value))
        return res
    h=get_basic_headers(text)
    b=to_body(text)
    return [('CONTENT_LENGTH',str(len(b)))] + h

def begin_request(keep_alive = 0):
    # responder 1 , keep_alive + padding
    content = '\0\1'+chr(keep_alive)+'\0' * 5
    return fcgi_record(BEGIN_REQUEST,content)

def to_fcgi_request(text,flags = 0):
    request = '';
    if flags & TEST_GET_VALUES:
        request+=fcgi_record(GET_VALUES,make_pairs([('FCGI_MPXS_CONNS',''),('FCGI_MAX_CONNS',''),('FCGI_MAX_REQS','')]),0)
    if flags & TEST_KEEP_ALIVE:
        request+=begin_request(1)
    else:
        request+=begin_request()
    headers = make_pairs(get_headers(text))
    if flags & TEST_RANDOM:
        while headers:
            l=random.randint(1,20)
            request+=fcgi_record(PARAMS,headers[:l])
            headers = headers[l:]
    else:
        request+=fcgi_record(PARAMS,headers)
    request+=fcgi_record(PARAMS,'')
    body = to_body(text);
    if body:
        request+=fcgi_record(STDIN,body)
    request+=fcgi_record(STDIN,'')
    return request

def from_fcgi_response(text,flags = 0):
    result = ''
    state = 0
    if flags & TEST_GET_VALUES:
        pairs = make_pairs([('FCGI_MPXS_CONNS','0'),('FCGI_MAX_CONNS','5'),('FCGI_MAX_REQS','5')])
        tmp = fcgi_record(GET_VALUES_RESULT,pairs,0)
        if text[:len(tmp)] != tmp:
            print 'Invalid get values result \n[%s]\n[%s]\n' % (escape(tmp) , escape(text[:len(tmp)]))
            sys.exit(1)
        text=text[len(tmp):]
    while text:
        header = text[:8]
        if len(header)!=8 :
            print 'Invalid header len'
            sys.exit(1)
        text = text[8:]
        type=ord(header[1])
        if header[0]!='\1' or header[2:4]!='\0\1' or header[7]!='\0':
            tmp=escape(header)
            print 'Invalid header [' + tmp + ']'
            sys.exit(1)
        clength = (ord(header[4]) << 8) + ord(header[5])
        plength = ord(header[6])
        if len(text) < clength + plength : 
            print 'Unexpected EOF'
            sys.exit(1)
        if (clength + plength) % 8 != 0:
            print 'Incorrect padding L=%d p=%d' % (clength,plength)
            sys.exit(1)
        content = text[:clength]
        text=text[clength + plength:]
        if state == 0:
            if type != STDOUT:
                print 'Unexpected type %d' % type
                sys.exit(1)
            if clength == 0:
                state = 1
            else:
                result +=content
        else:
            if type != END_REQUEST:
                print 'Unexpected type %d' % type
                sys.exit(1)
            if content != '\0'*8:
                print 'Unexpected End Request'
                sys.exit(1)
            if text:
                print 'Unexpected input'
            return result


