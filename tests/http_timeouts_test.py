#!/usr/bin/env python
# coding=utf-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
import socket
import time
import traceback
import random
import sys


timeout_time = 5

def test(x):
    if not x:
        print "Error"
        traceback.print_stack()
        sys.exit(1)

def make_sock():
    s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);
    s.connect(('localhost',8080));
    s.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
    return s;


def test_unfinished_out(msg,chunks=[]):
    print "Tesing %s with %d chunks" % (msg,len(chunks))
    s=make_sock();
    if len(msg) > 0 :
        s.send(msg)
    if len(chunks) > 0:
        for chunk in chunks:
            time.sleep(1)
            s.send(chunk)
    start = time.time()
    text = s.recv(1)
    passed  = time.time() - start
    test(len(text) == 0)
    global timeout_time
    test(passed > timeout_time - 2)
    test(passed < timeout_time + 2)

def test_unfinished_read(msg,reads,ignore):
    print "Tesing %s with %d reads" % (msg,reads)
    s=make_sock();
    read_size = s.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
    print "SO_RCVBUF=%d" % read_size
    if read_size < 32768:
        read_size = 32768
    if read_size > 128*1024:
        readsize = 65536
        s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF,readsize)
    s.send(msg + '\r\n\r\n')
    for n in xrange(0,reads):
        time.sleep(1)
        text = s.recv(read_size)
        l = len(text)
        test(l > 0)
    global timeout_time
    time.sleep(timeout_time * 2)
    n=0
    while 1:
        text = s.recv(read_size);
        if len(text) > 0:
            n = n + len(text)
        else:
            break
    test(n < read_size * 16 or n < 1000000)


write = sys.argv[1] == 'write'

if write:
    print 'Write to the client timeout'
    test_unfinished_read('GET /async/long HTTP/1.0',0,0)
    test_unfinished_read('GET /async/long HTTP/1.0',20,1000)
    test_unfinished_read('GET /sync/long HTTP/1.0',0,0)
    test_unfinished_read('GET /sync/long HTTP/1.0',20,1000)
else:
    print 'Read from client timeouts'
    test_unfinished_out('')
    test_unfinished_out('GET /')
    test_unfinished_out('POST / HTTP/1.0\r\nContent-Length:10000\r\n\r\nbla bla')
    test_unfinished_out('POST / HTTP/1.0\r\nContent-Length:10000\r\n\r\n', ['ss','ss','ss','ss'])

    print 'Disconnect the client timeout'

    test_unfinished_out('GET /async/goteof HTTP/1.0\r\n\r\n')

time.sleep(1)

