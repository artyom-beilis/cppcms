#!/usr/bin/env python
# coding=utf-8
#
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
#
import httplib
import sys
import re
import time
import datetime
import socket
import os

def test(x):
    if not x:
        raise RuntimeError("Failed")
def now():
    return "    " + datetime.datetime.now().strftime("%H:%M:%S.%f")

def make_content(first,length):
    all=[]
    for i in xrange(0,length):
        all.append(first)
        n=ord(first)+1
        if n > ord('z'):
            n=ord('a')
        first = chr(n)
    return ''.join(all)

def make_multipart_form_data(qs):
    r=[]
    fc={}
    ln={}
    post={}
    for item in qs:
        key=item.split('=')[0]
        value = item.split('=')[1]
        if key=='formdata':
            post[value]=True
        pr=key[0:2]
        if pr == 'f_' or pr == 'l_':
            name = key[2:]
            if pr == 'f_':
                fc[name]=value
            else:
                ln[name]=int(value)
            if not name in fc or not name in ln:
                continue
            l=ln[name]
            c=fc[name]
            r.append('--123456\r\n')
            if not name in post:
                r.append('Content-Type: text/plain\r\n')
            r.append('Content-Disposition: form-data; name="' + name +'"\r\n\r\n')
            r.append(make_content(c,l))
            r.append('\r\n')
    if len(r) != 0:
        r.append('--123456--\r\n')
    return ''.join(r)

class Conn:
    num=re.compile('^[0-9]+$')
    def __init__(self,path,q=[],custom_content=None):
        opts={}
        for item in q:
            sp=item.split('=')
            opts[sp[0]]=sp[1]
        if len(q)==0:
            self.path = path
        else:
            self.path = path + '?' + '&'.join(q)
        if custom_content:
            if type(custom_content) is str:
                post=custom_content;
                content_length=len(post)
                content_type='text/plain'
            else:
                post=custom_content['content']
                if 'content_length' in custom_content:
                    content_length=custom_content['content_length']
                else:
                    content_length=len(post)
                content_type=custom_content['content_type']
        else:
            post = make_multipart_form_data(q)
            content_length = len(post)
            content_type='multipart/form-data; boundary=123456'
        if content_length > 0:
            print 'POST ' + self.path
            query  = 'POST ' + self.path +' HTTP/1.0\r\n'
            query += ('Content-Type: %s\r\n' % content_type)
            query += ('Content-Length: %d\r\n\r\n' % content_length)
        else:
            print 'GET ' + self.path
            query = 'GET ' + self.path + ' HTTP/1.0\r\n\r\n'
        self.s=socket.socket(socket.AF_INET, socket.SOCK_STREAM);
        self.s.connect(('127.0.0.1',8080))
        self.s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        if 'chunks' in opts:
            size=int(opts['chunks'])
            self.s.send(query)
            pos = 0;
            while pos < content_length:
                if pos > 0:
                    time.sleep(0.05)
                chunk = post[pos:pos+size]
                pos+=size
                self.s.send(chunk)
        else:
            self.s.send(query + post)

    def get(self):
        response = ''
        while True:
            tmp=self.s.recv(1000)
            if len(tmp) == 0:
                self.s.close()
                break
            response = response + tmp
        r2 = response.split('\r\n\r\n')
        headers=r2[0]
        if len(r2) == 2:
            body = r2[1]
        else:
            body = ''
        first_header = headers.split('\r\n')[0]
        if first_header.find('HTTP/1.0 ')==0:
            status=int(first_header[9:12])
        else:
            status=0
        result =  {'status' : status }
        result['is_html'] = body.strip().find('<')==0
        if result['is_html']:
            result['content']=body
        else:
            items=body.split('\n')
            for item in items:
                kv=item.split('=')
                if len(kv)!=2:
                    continue
                if Conn.num.match(kv[1]):
                    result[kv[0]]=int(kv[1])
                else:
                    result[kv[0]]=kv[1]
        print 'Got %s ' % result
        return result

def transfer(path,q=[],custom_content=None):
    c=Conn(path,q,custom_content)
    return c.get()
    
def test_on_error_called(expected=1):
    test(transfer('/upload/total_on_error',[])['total_on_error']==expected)

def test_upload():
    r=transfer('/upload/no_content',[])
    test(r['no_content']==1)
    test(r['status']==200)
    r=transfer('/upload',['l_1=10','f_1=a','l_2=20','f_2=t'])
    test(r['files']==2)
    test(r['total']==2)
    test(r['on_new_file']==2)
    test(r['on_upload_progress']==0)
    r=transfer('/upload',['l_1=10','f_1=a','l_2=20','f_2=t','chunks=5'])
    test(r['files']==2)
    test(r['total']==2)
    test(r['on_new_file']==2)
    test(r['on_upload_progress']>2)
    r=transfer('/upload',['l_1=10000','f_1=a','l_2=20000','f_2=t'])
    test(r['files']==2)
    test(r['total']==2)
    test(r['on_new_file']==2)
    test(r['on_upload_progress']>2)

    code=500
    for loc in ['on_headers_ready','on_new_file','on_upload_progress','on_data_ready','on_end_of_content']:
        code=code+1
        for how in [0,1,2,3]:
            r=transfer('/upload',['l_1=50','f_1=a','abort=%s' % loc,'how=%d' % how,'setbuf=10','at=1'])
            test(r['status']==code)
            if how == 0:
                test(r['is_html'])
            else:
                test(r['at']==loc)
    
    r=transfer('/upload',[],{'content' : 'a=b&foo=bar','content_type':'application/x-www-form-urlencoded'})
    test(r['files']==0)
    test(r['on_new_file']==0)
    test(r['on_upload_progress']==0)
    test(r['on_end_of_content']==1)
    test(r['post']==2)

    r=transfer('/upload',['fail=1'],{'content' : 'a=b','content_length':4,'content_type':'application/x-www-form-urlencoded'})
    test_on_error_called()
    r=transfer('/upload',['fail=1'],{'content' : '1234567890','content_type':'multipart/form-data; boundary=123456'})
    test(r['status']==400)
    test_on_error_called()
    
    try:
        os.remove('test.txt')
    except:
        pass
    r=transfer('/upload',['l_1=100','f_1=a','save_to_1=test.txt'])
    test(r['status']==200)
    time.sleep(0.2);
    test(open('test.txt','rb').read() == make_content('a',100))
    os.remove('test.txt')

    test(transfer('/upload',['l_1=100','f_1=a','cl_limit=5'])['status']==200)
    test(transfer('/upload',['fail=1','cl_limit=5'],{'content':'{"x":1000}','content_type':'application/json'})['status']==413)
    test_on_error_called()
    test(transfer('/upload',['fail=1','l_1=100','f_1=a','mp_limit=50'])['status']==413)
    test_on_error_called()
    test(transfer('/upload',['l_1=100','f_1=a','mp_limit=200'])['status']==200)
    test(transfer('/upload',['formdata=1','l_1=100','f_1=a','l_2=200','f_2=b','mp_limit=500','cl_limit=100'])['status']==200)
    test(transfer('/upload',['fail=1','formdata=1','l_1=100','f_1=a','l_2=200','f_2=b','mp_limit=500','cl_limit=99'])['status']==413)
    test_on_error_called()

def test_raw():
    r=transfer('/raw/no_content',[])
    test(r['no_content']==1)
    test(r['status']==200)
    r=transfer('/raw',['l_1=10','f_1=a'],make_content('a',10))
    test(r['status']==200)
    r=transfer('/raw',['l_1=10','f_1=a','chunks=5'],make_content('a',10))
    test(r['status']==200)
    test(r['on_data_chunk']==2)
    r=transfer('/raw',['l_1=10000','f_1=a'],make_content('a',10000))
    test(r['status']==200)
    test(r['on_data_chunk']>=2)
    r=transfer('/raw',['l_1=10000','f_1=a','setbuf=10'],make_content('a',10000))
    test(r['status']==200)
    test(r['on_data_chunk']>=100)

    code=500
    for loc in ['on_headers_ready','on_data_chunk','on_end_of_content']:
        code=code+1
        for how in [0,1,2,3]:
            for at in [0,20]:
                r=transfer('/raw',['l_1=50','f_1=a','abort=%s' % loc,'how=%d' % how,'setbuf=10','at=%d' % at],make_content('a',50))
                test(r['status']==code)
                if how == 0:
                    test(r['is_html'])
                else:
                    test(r['at']==loc)
        
    r=transfer('/raw',['fail=1'],{'content' : 'a=b','content_length':4,'content_type':'application/x-www-form-urlencoded'})
    test_on_error_called()
    r=transfer('/raw',[],{'content' : 'abcdefghig','content_type':'multipart/form-data; boundary=123456'})
    test(r['status']==200)
    
    test(transfer('/raw',['l_2=100','f_2=a','cl_limit=5'])['status']==200)
    test(transfer('/raw',['fail=1','cl_limit=5'],{'content':'{"x":1000}','content_type':'application/json'})['status']==413)
    test_on_error_called()
    test(transfer('/raw',['fail=1','l_2=100','f_2=a','mp_limit=50'])['status']==413)
    test_on_error_called()
    test(transfer('/raw',['l_2=100','f_2=a','mp_limit=200'])['status']==200)
    test(transfer('/raw',['formdata=1','l_3=100','f_3=a','l_2=200','f_2=b','mp_limit=500','cl_limit=100'])['status']==200)
    test(transfer('/raw',['formdata=1','l_3=100','f_3=a','l_2=200','f_2=b','mp_limit=500','cl_limit=99'])['status']==200)


test_upload()
test_raw()
test_on_error_called(0)
print "OK"
