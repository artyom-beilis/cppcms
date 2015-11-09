#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

from ctypes import *
import sys
import types

capi=cdll.LoadLibrary(sys.argv[1]);


class MManip:
    def __init__(self,c,cname):
        self.Class = c;
        self.cname = cname;
    def add_method(self,name,result,params,check_error=True):
        real_method = getattr(capi,self.cname + "_" + name);
        real_method.restype = result
        real_method.argstypes = params
        if name == 'new':
            def wrap(self):
                self.d=real_method()
            setattr(self.Class,name,classmethod(wrap))
        else:
            def wrap(self,*args):
                r=real_method(self.d,*args)
                if check_error:
                    if(self.got_error()):
                        raise RuntimeError(self.strerror())
                return r
            setattr(self.Class,name,classmethod(wrap))

class SessionPool:
    def __init__(self):
        self.new()
    def __del__(self):
        self.delete()
    @staticmethod
    def setup():
        m=MManip(SessionPool,"cppcms_capi_session_pool")
        m.add_method('new',c_void_p,[],False)
        m.add_method('delete',None,[c_void_p],False)
        m.add_method('strerror',c_char_p,[c_void_p],False)
        m.add_method('got_error',c_int,[c_void_p],False)
        m.add_method('clear_error',None,[c_void_p],False)
        m.add_method('init',c_int,[c_char_p])
        m.add_method('init_from_json',c_int,[c_char_p])

SessionPool.setup()


def x():
    p=SessionPool()
    p.init(sys.argv[2])

x()


