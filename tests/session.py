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
    def add_method(self,name,result,params,check_error=True,mname=None):
        if not mname:
            mname = name
        real_method = getattr(capi,self.cname + "_" + name);
        real_method.restype = result
        real_method.argstypes = params
        cname = self.cname
        if name == 'new':
            def wrap(self):
                self.d=real_method()
                #print 'Calling %s.%s() -> %s' % (cname,name,self.d)
            setattr(self.Class,mname,wrap)
        else:
            def wrap(self,*args):
                r=real_method(self.d,*args)
                #print 'Calling %s->%s.%s(%s) -> %s' % (self.d,cname,name,list(args),r)
                if check_error:
                    if(self.got_error()):
                        raise RuntimeError(self.strerror())
                return r
            setattr(self.Class,mname,wrap)

class SessionPool:
    def __init__(self):
        self.new()
    def __del__(self):
        self.impl_delete()
    @staticmethod
    def setup():
        m=MManip(SessionPool,"cppcms_capi_session_pool")
        m.add_method('new',c_void_p,[],False)
        m.add_method('delete',None,[c_void_p],False,'impl_delete')
        m.add_method('strerror',c_char_p,[c_void_p],False)
        m.add_method('got_error',c_int,[c_void_p],False)
        m.add_method('clear_error',None,[c_void_p],False)
        m.add_method('init',c_int,[c_char_p])
        m.add_method('init_from_json',c_int,[c_char_p])

SessionPool.setup()

class Cookie:
    def __init__(self,ptr):
        self.d=ptr
    def __del__(self):
        self.impl_delete()
    def __str__(self):
        r=self.header()
        if r:
            return r
        else:
            return "deleted cookie"
    @staticmethod
    def setup():
        m=MManip(Cookie,"cppcms_capi_cookie")
        m.add_method('delete',None,[c_void_p],False,'impl_delete')
        m.add_method('name',c_char_p,[c_void_p],False)
        m.add_method('value',c_char_p,[c_void_p],False)
        m.add_method('path',c_char_p,[c_void_p],False)
        m.add_method('domain',c_char_p,[c_void_p],False)
        m.add_method('header',c_char_p,[c_void_p],False)
        m.add_method('header_content',c_char_p,[c_void_p],False)
        m.add_method('max_age',c_uint,[c_void_p],False)
        m.add_method('max_age_defined',c_int,[c_void_p],False)
        m.add_method('expires',c_longlong,[c_void_p],False)
        m.add_method('expires_defined',c_int,[c_void_p],False)
        m.add_method('is_secure',c_int,[c_void_p],False)

Cookie.setup()

class Session:
    def __init__(self,pool):
        self.new()
        self.impl_init(pool.d)
    def __del__(self):
        self.impl_delete()    
        pass
    
    class KeyIterator:
        def __init__(self,session):
            self.s = session
            self.started = False
        def __iter__(self):
            return self
        def next(self):
            if self.started:
                next_key = self.s.get_next_key()
            else:
                next_key = self.s.get_first_key()
                self.started = True
            if not next_key:
                raise StopIteration
            return next_key
    def keys(self):
        return Session.KeyIterator(self)

    class CookieIterator:
        def __init__(self,session):
            self.s = session
            self.started = False
        def __iter__(self):
            return self
        def next(self):
            if self.started:
                r = self.s.cookie_next()
            else:
                r = self.s.cookie_first()
                self.started = True
            if not r:
                raise StopIteration
            return r
    def keys(self):
        return Session.KeyIterator(self)
    def cookies(self):
        return Session.CookieIterator(self)



    @staticmethod
    def setup():
        m=MManip(Session,"cppcms_capi_session")
        m.add_method('new',c_void_p,[],False)
        m.add_method('delete',None,[c_void_p],False,'impl_delete')
        m.add_method('strerror',c_char_p,[c_void_p],False)
        m.add_method('got_error',c_int,[c_void_p],False)
        m.add_method('clear_error',None,[c_void_p],False)
        m.add_method('init',c_int,[c_char_p],True,'impl_init')
        m.add_method('clear',c_int,[c_void_p])
        m.add_method('is_set',c_int,[c_void_p,c_char_p])
        m.add_method('erase',c_int,[c_void_p,c_char_p])
        m.add_method('get_exposed',c_int,[c_void_p,c_char_p])
        m.add_method('set_exposed',c_int,[c_void_p,c_char_p,c_int])
        m.add_method('get_first_key',c_char_p,[c_void_p])
        m.add_method('get_next_key',c_char_p,[c_void_p])
        m.add_method('get_csrf_token',c_char_p,[c_void_p])
        m.add_method('get',c_char_p,[c_void_p,c_char_p])
        m.add_method('set',c_int,[c_void_p,c_char_p,c_char_p,c_int],True,'impl_set')
        m.add_method('get_session_cookie_name',c_char_p,[c_void_p])
        m.add_method('save',c_int,[c_void_p])
        m.add_method('load',c_int,[c_void_p,c_char_p])
        m.add_method('cookie_first',c_void_p,[c_void_p],True,'impl_cookie_first')
        m.add_method('cookie_next',c_void_p,[c_void_p],True,'impl_cookie_next')

    def set(self,key,value):
        self.impl_set(key,value,-1)

    def cookie_first(self):
        p=self.impl_cookie_first();
        if p:
            return Cookie(p)
        return None
    def cookie_next(self):
        p=self.impl_cookie_next();
        if p:
            return Cookie(p)
        return None


Session.setup()


def test():
    state=''
    p=SessionPool()
    p.init(sys.argv[2])
    s=Session(p)
    s.load(state)
    s.set('x','111')
    s.set('y','222')
    s.set_exposed('x',1)
    for k in s.keys():
        print 'Got ' + k 
        print 'Value ' + s.get(k)
    s.save()
    for c in s.cookies():
        print c
        print c.value()
        if(c.name()==s.get_session_cookie_name()):
            state = c.value()
    s=None
    s=Session(p)
    s.load(state)
    s.set_exposed('x',0)
    s.save()
    for c in s.cookies():
        print c
        print c.value()
        if(c.name()==s.get_session_cookie_name()):
            state = c.value()
    s=None
    s=Session(p)
    s.load(state)
    s.clear()
    s.save()
    for c in s.cookies():
        print c
        print c.value()
        if(c.name()==s.get_session_cookie_name()):
            state = c.value()

test()


