#!/usr/bin/env python
# coding=utf-8
#
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
#

import httpclient
import httplib
import sys



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



print "- Testing CSRF"
session = httpclient.Session(print_cookies = False)
csrf = session.transmit('/gettoken')
test(session.transmit('/post',post_data = '_csrf=%s&test=value' % csrf )=='ok')
test(session.transmit('/post',post_data = 'test=value',headers = {'X-CSRFToken' : csrf })=='ok')
test(session.transmit('/post',post_data = '_csrf=invalid&test=value')=='fail')
test(session.transmit('/post',post_data = 'test=value',headers = {'X-CSRFToken' : 'invalid' })=='fail')
test(session.transmit('/post',post_data = 'test=value')=='fail')
test(session.transmit('/post?test=value')=='ok')
session = httpclient.Session(print_cookies = False)
test(session.transmit('/post',post_data = 'test=value')=='ok')

print "- Testing Content-Length limits"

test(session.transmit('/post',post_data = 'test=' + 'x'*(1024-5)) == 'ok')
session.transmit('/post',post_data = 'test=' + 'x'*(1024-4));
test(session.status==413)

print "- Testing Upload limits"

empty_length=len(make_multipart_form_data(''))

normal_file_data1 = make_multipart_form_data('x'*(2048 - empty_length));
normal_file_data2 = make_multipart_form_data('x' * 1024,None,None);
big_file_data1 = make_multipart_form_data('x'*(2048 - empty_length + 1 ));
big_file_data2 = make_multipart_form_data('x' * 1025,None,None);
bad_file_data1 = normal_file_data1[0:300]


# HTTP

session = httpclient.Session(print_cookies = False)


test(session.transmit('/post',post_data = normal_file_data1,content_type = 'multipart/form-data; boundary=123456') == 'ok')
test(session.transmit('/post',post_data = normal_file_data2,content_type = 'multipart/form-data; boundary=123456') == 'ok')
session.transmit('/post',post_data = big_file_data1,content_type = 'multipart/form-data; boundary=123456')
test(session.status==413)
session.status = 0
session.transmit('/post',post_data = big_file_data2,content_type = 'multipart/form-data; boundary=123456')
test(session.status==413)
session.transmit('/post',post_data = normal_file_data1,content_type = 'multipart/form-data; boundary=abcde')
test(session.status==400)
session.status = 0
session.transmit('/post',post_data = normal_file_data1,content_type = 'multipart/form-data')
test(session.status==400)
session.status = 0
session.transmit('/post',post_data = bad_file_data1,content_type = 'multipart/form-data; boundary=123456')
test(session.status==400)


