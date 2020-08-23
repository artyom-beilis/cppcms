#!/usr/bin/env python
# coding=utf-8
#
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
#
from __future__ import print_function
import httpclient
try:
    import httplib
except:
    import http.client as httplib
import sys
import socket

def test(X):
    if not X:
        raise Exception('Test failed')


def make_multipart_form_data(content,mime='text/plain',name='test.txt'):
    r= '--123456\r\n'
    if mime:
        r += 'Content-Type: ' + mime + '\r\n'
    if name:
        r+= 'Content-Disposition: form-data; name="file"; filename="' + name +'"\r\n'
    else:
        r+= 'Content-Disposition: form-data; name="file"\r\n'
    r+=  '\r\n' +  content + '\r\n--123456--\r\n' 
    return r



print("- Testing CSRF")
session = httpclient.Session(print_cookies = False)
csrf = session.transmit('/gettoken').decode()
test(session.transmit('/post',post_data = '_csrf=%s&test=value' % csrf )==b'ok')
test(session.transmit('/post',post_data = 'test=value',headers = {'X-CSRFToken' : csrf })==b'ok')
test(session.transmit('/post',post_data = '_csrf=invalid&test=value')==b'fail')
test(session.transmit('/post',post_data = 'test=value',headers = {'X-CSRFToken' : 'invalid' })==b'fail')
test(session.transmit('/post',post_data = 'test=value')==b'fail')
test(session.transmit('/post?test=value')==b'ok')
session = httpclient.Session(print_cookies = False)
test(session.transmit('/post',post_data = 'test=value')==b'ok')

print("- Testing Content-Length limits")

test(session.transmit('/post',post_data = 'test=' + 'x'*(1024-5)) == b'ok')
try:
    session.status = 0
    session.transmit('/post',post_data = 'test=' + 'x'*(1024-4));
    test(session.status==413 )
except socket.error:
    print("-- got socket.error")

print("- Testing Upload limits")

empty_length=len(make_multipart_form_data(''))

normal_file_data1 = make_multipart_form_data('x'*(2048 - empty_length));
normal_file_data2 = make_multipart_form_data('x' * 1024,None,None);
big_file_data1 = make_multipart_form_data('x'*(2048 - empty_length + 1 ));
big_file_data2 = make_multipart_form_data('x' * 1025,None,None);
bad_file_data1 = normal_file_data1[0:300]


# HTTP

session = httpclient.Session(print_cookies = False)


test(session.transmit('/post',post_data = normal_file_data1,content_type = 'multipart/form-data; boundary=123456') == b'ok')
test(session.transmit('/post',post_data = normal_file_data2,content_type = 'multipart/form-data; boundary=123456') == b'ok')

try:
    session.status = 0
    session.transmit('/post',post_data = big_file_data1,content_type = 'multipart/form-data; boundary=123456')
    test(session.status==413)
except socket.error:
    print("-- got socket.error")


try:
    session.status = 0
    session.transmit('/post',post_data = big_file_data2,content_type = 'multipart/form-data; boundary=123456')
    test(session.status==413)
except socket.error:
    print("-- got socket.error")


try:
    session.status = 0
    session.transmit('/post',post_data = normal_file_data1,content_type = 'multipart/form-data; boundary=abcde')
    test(session.status==400)
except socket.error:
    print("-- got socket.error")

try:
    session.status = 0
    session.transmit('/post',post_data = normal_file_data1,content_type = 'multipart/form-data')
    test(session.status==400)
except socket.error:
    print("-- got socket.error")


try:
    session.status = 0
    session.transmit('/post',post_data = bad_file_data1,content_type = 'multipart/form-data; boundary=123456')
    test(session.status==400)
except socket.error:
    print("-- got socket.error")


