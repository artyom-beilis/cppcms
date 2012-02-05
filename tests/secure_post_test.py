#!/usr/bin/env python
# coding=utf-8
#
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
#
from httpclient import Session, Cookie
import httplib
import sys



def test(X):
    if not X:
        raise Exception('Test failed')


def make_multipart_form_data(content,mime='text/plain',name='test.txt'):
    return \
      '--123456\r\n' + \
      'Content-Type: ' + mime + '\r\n' + \
      'Content-Disposition: form-data; name="file"; filename="' + name +'"\r\n' + \
      '\r\n' + \
      content + \
      '\r\n--123456--\r\n' 



print "Testing CSRF"
session  = Session()
csrf = session.transmit('/gettoken')
test(session.transmit('/post',post_data = '_csrf=%s&test=value' % csrf )=='ok')
test(session.transmit('/post',post_data = 'test=value',headers = {'X-CSRFToken' : csrf })=='ok')
test(session.transmit('/post',post_data = '_csrf=invalid&test=value')=='fail')
test(session.transmit('/post',post_data = 'test=value',headers = {'X-CSRFToken' : 'invalid' })=='fail')
test(session.transmit('/post',post_data = 'test=value')=='fail')
test(session.transmit('/post?test=value')=='ok')
session = Session()
test(session.transmit('/post',post_data = 'test=value')=='ok')

print "Testing Content-Length limits"

test(session.transmit('/post',post_data = 'test=' + 'x'*(1024-5)) == 'ok')
try:
    test(session.transmit('/post',post_data = 'test=' + 'x'*(1024-4)) == '')
    test(session.status==413)
except httplib.BadStatusLine:
    pass

print "Testing Upload limits"

empty_length=len(make_multipart_form_data(''))
file_data = make_multipart_form_data('x'*(2048 - empty_length));
test(session.transmit('/post',post_data = file_data,content_type = 'multipart/form-data; boundary=123456') == 'ok')
file_data = make_multipart_form_data('x'*(2048 - empty_length + 1 ));
try:
    test(session.transmit('/post',post_data = file_data,content_type = 'multipart/form-data; boundary=123456') == '')
    test(session.status==413)
except httplib.BadStatusLine:
    pass

