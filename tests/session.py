#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

from ctypes import *
import sys

capi=cdll.LoadLibrary(sys.argv[1]);
capi.cppcms_capi_session_pool_new.restype=c_void_p

capi.cppcms_capi_session_pool_delete.argtypes = [ c_void_p ]

capi.cppcms_capi_session_pool_strerror.restype =c_char_p
capi.cppcms_capi_session_pool_strerror.argtypes = [ c_void_p ]

capi.cppcms_capi_session_pool_init.restype=c_int
capi.cppcms_capi_session_pool_init.argtype= [ c_void_p ,c_char_p ]

capi.cppcms_capi_session_new.restype = c_void_p
capi.cppcms_capi_session_delete.argtypes = [ c_void_p ]
capi.cppcms_capi_session_init.argtypes = [ c_void_p,c_void_p ]
capi.cppcms_capi_session_strerror.restype =c_char_p
capi.cppcms_capi_session_strerror.argtypes = [ c_void_p ]
capi.cppcms_capi_session_get_session_cookie_name.restype = c_char_p
capi.cppcms_capi_session_get_session_cookie_name.argtypes = [ c_void_p ]


class SessionPool:
    def __init__(self,path) :
        self.d = capi.cppcms_capi_session_pool_new();
        if capi.cppcms_capi_session_pool_init(self.d,path) != 0:
            err = capi.cppcms_capi_session_pool_strerror(self.d);
            capi.cppcms_capi_session_pool_delete(self.d);
            self.d=None
            raise RuntimeError(err)
    def __del__(self):
        capi.cppcms_capi_session_pool_delete(self.d)

class SessionInterface:
    def __init__(self,pool):
        self.d = capi.cppcms_capi_session_new()
        if capi.cppcms_capi_session_init(self.d,pool.d)!=0:
            err = capi.cppcms_capi_session_strerror(self.d)
            capi.cppcms_capi_session_delete(self.d)
            self.d=None
            raise RuntimeError(err)
    def get_session_cookie_name(self):
        return capi.cppcms_capi_session_get_session_cookie_name(self.d)
    def set(self,key,value):
        capi.cppcms_capi_session_set(self.d,key,value)
    def get(self,key):
        return capi.cppcms_capi_session_get(self.d,key)
    def load(self,cookie):
        capi.cppcms_capi_session_load(self.d,cookie)
    def save(self):
        capi.cppcms_capi_session_save(self.d)
    def __del__(self):
        capi.cppcms_capi_session_delete(self.d)

def x():
    pass
    #p=SessionPool(sys.argv[2])
x()
print "A"
capi = None
sys.exit(0)
print "B"


