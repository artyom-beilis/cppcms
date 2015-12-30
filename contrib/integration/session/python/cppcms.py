#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

from ctypes import *
import sys
import types
import threading
from datetime import datetime

class Loader:
    """The native cppcms library access API"""
    lock = threading.Lock()
    capi = None
    SESSION_FIXED=0
    SESSION_RENEW=1
    SESSION_BROWSER=2
    ERROR_OK=0
    ERROR_GENERAL=1
    ERROR_RUNTIME=2
    ERROR_INVALID_ARGUMENT=4
    ERROR_LOGIC=5
    ERROR_ALLOC=6
    @classmethod
    def configAPI(cls):
        cls.capi.cppcms_capi_error.restype=c_int
        cls.capi.cppcms_capi_error.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_error_message.restype=c_char_p
        cls.capi.cppcms_capi_error_message.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_error_clear.restype=c_char_p
        cls.capi.cppcms_capi_error_clear.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_pool_new.restype=c_void_p
        cls.capi.cppcms_capi_session_pool_new.argtypes=[  ]
        cls.capi.cppcms_capi_session_pool_delete.restype=None
        cls.capi.cppcms_capi_session_pool_delete.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_pool_init.restype=c_int
        cls.capi.cppcms_capi_session_pool_init.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_pool_init_from_json.restype=c_int
        cls.capi.cppcms_capi_session_pool_init_from_json.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_new.restype=c_void_p
        cls.capi.cppcms_capi_session_new.argtypes=[  ]
        cls.capi.cppcms_capi_session_delete.restype=None
        cls.capi.cppcms_capi_session_delete.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_init.restype=c_int
        cls.capi.cppcms_capi_session_init.argtypes=[ c_void_p,c_void_p ]
        cls.capi.cppcms_capi_session_clear.restype=c_int
        cls.capi.cppcms_capi_session_clear.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_is_set.restype=c_int
        cls.capi.cppcms_capi_session_is_set.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_erase.restype=c_int
        cls.capi.cppcms_capi_session_erase.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_get_exposed.restype=c_int
        cls.capi.cppcms_capi_session_get_exposed.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_set_exposed.restype=c_int
        cls.capi.cppcms_capi_session_set_exposed.argtypes=[ c_void_p,c_char_p,c_int ]
        cls.capi.cppcms_capi_session_get_first_key.restype=c_char_p
        cls.capi.cppcms_capi_session_get_first_key.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_get_next_key.restype=c_char_p
        cls.capi.cppcms_capi_session_get_next_key.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_get_csrf_token.restype=c_char_p
        cls.capi.cppcms_capi_session_get_csrf_token.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_set.restype=c_int
        cls.capi.cppcms_capi_session_set.argtypes=[ c_void_p,c_char_p,c_char_p ]
        cls.capi.cppcms_capi_session_get.restype=c_char_p
        cls.capi.cppcms_capi_session_get.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_set_binary_as_hex.restype=c_int
        cls.capi.cppcms_capi_session_set_binary_as_hex.argtypes=[ c_void_p,c_char_p,c_char_p ]
        cls.capi.cppcms_capi_session_get_binary_as_hex.restype=c_char_p
        cls.capi.cppcms_capi_session_get_binary_as_hex.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_set_binary.restype=c_int
        cls.capi.cppcms_capi_session_set_binary.argtypes=[ c_void_p,c_char_p,c_void_p,c_int ]
        cls.capi.cppcms_capi_session_get_binary.restype=c_int
        cls.capi.cppcms_capi_session_get_binary.argtypes=[ c_void_p,c_char_p,c_void_p,c_int ]
        cls.capi.cppcms_capi_session_get_binary_len.restype=c_int
        cls.capi.cppcms_capi_session_get_binary_len.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_reset_session.restype=c_int
        cls.capi.cppcms_capi_session_reset_session.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_set_default_age.restype=c_int
        cls.capi.cppcms_capi_session_set_default_age.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_set_age.restype=c_int
        cls.capi.cppcms_capi_session_set_age.argtypes=[ c_void_p,c_int ]
        cls.capi.cppcms_capi_session_get_age.restype=c_int
        cls.capi.cppcms_capi_session_get_age.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_set_default_expiration.restype=c_int
        cls.capi.cppcms_capi_session_set_default_expiration.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_set_expiration.restype=c_int
        cls.capi.cppcms_capi_session_set_expiration.argtypes=[ c_void_p,c_int ]
        cls.capi.cppcms_capi_session_get_expiration.restype=c_int
        cls.capi.cppcms_capi_session_get_expiration.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_set_on_server.restype=c_int
        cls.capi.cppcms_capi_session_set_on_server.argtypes=[ c_void_p,c_int ]
        cls.capi.cppcms_capi_session_get_on_server.restype=c_int
        cls.capi.cppcms_capi_session_get_on_server.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_get_session_cookie_name.restype=c_char_p
        cls.capi.cppcms_capi_session_get_session_cookie_name.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_set_session_cookie.restype=c_int
        cls.capi.cppcms_capi_session_set_session_cookie.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_add_cookie_name.restype=c_int
        cls.capi.cppcms_capi_session_add_cookie_name.argtypes=[ c_void_p,c_char_p ]
        cls.capi.cppcms_capi_session_load.restype=c_int
        cls.capi.cppcms_capi_session_load.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_save.restype=c_int
        cls.capi.cppcms_capi_session_save.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_cookie_first.restype=c_void_p
        cls.capi.cppcms_capi_session_cookie_first.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_session_cookie_next.restype=c_void_p
        cls.capi.cppcms_capi_session_cookie_next.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_delete.restype=None
        cls.capi.cppcms_capi_cookie_delete.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_header.restype=c_char_p
        cls.capi.cppcms_capi_cookie_header.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_header_content.restype=c_char_p
        cls.capi.cppcms_capi_cookie_header_content.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_name.restype=c_char_p
        cls.capi.cppcms_capi_cookie_name.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_value.restype=c_char_p
        cls.capi.cppcms_capi_cookie_value.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_path.restype=c_char_p
        cls.capi.cppcms_capi_cookie_path.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_domain.restype=c_char_p
        cls.capi.cppcms_capi_cookie_domain.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_max_age_defined.restype=c_int
        cls.capi.cppcms_capi_cookie_max_age_defined.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_max_age.restype=c_uint
        cls.capi.cppcms_capi_cookie_max_age.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_expires_defined.restype=c_int
        cls.capi.cppcms_capi_cookie_expires_defined.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_expires.restype=c_longlong
        cls.capi.cppcms_capi_cookie_expires.argtypes=[ c_void_p ]
        cls.capi.cppcms_capi_cookie_is_secure.restype=c_int
        cls.capi.cppcms_capi_cookie_is_secure.argtypes=[ c_void_p ]
    
    @classmethod
    def load_from(cls,lst):
        for name in lst:
            try:
                cls.capi=cdll.LoadLibrary(name)
            except:
                cls.capi=None
            if cls.capi != None:
                return
        raise RuntimeError('Failed to load any of ' + ', '.join(lst))
    @classmethod
    def load(cls,path=None):
        """Initialize API by loading libcppcms from the given path"""
        with cls.lock:
            if cls.capi!=None:
                return
            if path==None:
                import platform
                system = platform.system().lower()
                if system=='windows':
                    cls.load_from(['cppcms.dll','libcppcms.dll','cppcms-1.dll','libcppcms-1.dll'])
                elif system=='macosx' or system=='darwin':
                    cls.load_from(['libcppcms.dylib','libcppcms.1.dylib'])
                elif system.find('cygwin')!=-1 :
                    cls.load_from(['cygcppcms.dll','cygcppcms-1.dll'])
                else:
                    cls.load_from(['libcppcms.so','libcppcms.so.1'])
            else:
                cls.capi = cdll.LoadLibrary(path);
            cls.configAPI()

class SessionBase:
    def check(self):
        code = Loader.capi.cppcms_capi_error(self.d)
        if code !=0:
            msg = Loader.capi.cppcms_capi_error_clear(self.d)
            if code == Loader.ERROR_ALLOC:
                raise MemoryError(msg)
            if code == Loader.ERROR_INVALID_ARGUMENT or code == Loader.ERROR_LOGIC:
                raise ValueError(msg)
            else:
                raise RuntimeError(msg)

class SessionPool(SessionBase):
    """Pool that handles all session - should be single instance per application """
    def __init__(self,config_file=None,json_text=None):
        Loader.load()
        """Initialize the pool from either CppCMS configuration file or directly from JSON text"""
        if config_file == None and json_text == None:
            raise ValueError('either config_file or json_text should be provided')
        if config_file != None and json_text != None:
            raise ValueError('Both config_file are json_text specified')
        self.d=Loader.capi.cppcms_capi_session_pool_new()
        if config_file!=None:
            Loader.capi.cppcms_capi_session_pool_init(self.d,config_file)
        else:
            Loader.capi.cppcms_capi_session_pool_init_from_json(self.d,json_text)
        try:
            self.check()
        except:
            Loader.capi.cppcms_capi_session_pool_delete(self.d)
            self.d=None
            raise
    def __del__(self):
        Loader.capi.cppcms_capi_session_pool_delete(self.d)
    def session(self):
        """Get session for use with request/response - thread safe method"""
        return Session(self)

class Cookie:
    """Http Cookie that is generated when session is saved"""
    def __init__(self,ptr):
        """Internal API never use directly"""
        self.d=ptr
    def __del__(self):
        Loader.capi.cppcms_capi_cookie_delete(self.d)
    def __str__(self):
        return self.header()
    def name(self):
        return Loader.capi.cppcms_capi_cookie_name(self.d)
    def value(self):
        return Loader.capi.cppcms_capi_cookie_value(self.d)
    def domain(self):
        return Loader.capi.cppcms_capi_cookie_domain(self.d)
    def header_content(self):
        """
        Returns a header content, i.e. stuff after Set-Cookie:
        for example:  'name=cppcms_session; value=I2343243252; Version=1' 
        """
        return Loader.capi.cppcms_capi_cookie_header_content(self.d)
    def header(self):
        """
        returns full set cookie header, for example:
        'Set-Cookie: name=cppcms_session; value=I2343243252; Version=1'
        """
        return Loader.capi.cppcms_capi_cookie_header(self.d)
    def path(self):
        return Loader.capi.cppcms_capi_cookie_path(self.d)
    def max_age(self):
        """Returns max-age value, note check max_age_defined before using it"""
        return Loader.capi.cppcms_capi_cookie_max_age(self.d)
    def expires(self):
        """Returns expires value, note check expires_defined before using it"""
        return Loader.capi.cppcms_capi_cookie_expires(self.d)
    def expires_defined(self):
        """Returns if expires present in the cookie"""
        return Loader.capi.cppcms_capi_cookie_expires_defined(self.d)
    def max_age_defined(self):
        """Returns if max-age present in the cookie"""
        return Loader.capi.cppcms_capi_cookie_max_age_defined(self.d)
    def is_secure(self):
        return Loader.capi.cppcms_capi_cookie_is_secure(self.d)

class Session(SessionBase):
    """
    Object that is used to update current CppCMS session
    Note once the session is loaded it can be accesses
    as dictionary to modify its keys and values
    """
    def __init__(self,pool):
        self.d=Loader.capi.cppcms_capi_session_new()
        Loader.capi.cppcms_capi_session_init(self.d,pool.d)
        try:
            self.check()
        except:
            Loader.capi.cppcms_capi_session_delete(self.d)
            self.d=None
            raise
    def __del__(self):
        Loader.capi.cppcms_capi_session_delete(self.d)
    def clear(self):
        """Clear entire session"""
        Loader.capi.cppcms_capi_session_clear(self.d)
        self.check()
    def is_set(self,key):
        """Check if key is defined"""
        r=Loader.capi.cppcms_capi_session_is_set(self.d,key)
        self.check()
        return r; 
    def erase(self,key):
        """Remove key from session"""
        Loader.capi.cppcms_capi_session_erase(self.d,key)
        self.check()
    def get_exposed(self,key):
        """Check if the key's value is exposed in cookies"""
        r=Loader.capi.cppcms_capi_session_get_exposed(self.d,key)
        self.check()
        return r!=0; 
    def set_exposed(self,key,v):
        """Set if the key's value should be exposed in cookies"""
        Loader.capi.cppcms_capi_session_set_exposed(self.d,key,v)
        self.check()

    def keys(self):
        """Get list of all keys"""
        l=[]
        r=Loader.capi.cppcms_capi_session_get_first_key(self.d)
        while r:
            l.append(r)
            r=Loader.capi.cppcms_capi_session_get_next_key(self.d)
        self.check()
        return l
    def cookies(self):
        """Get all cookies defined when the session is saved - must be called after calling save"""
        l=[]
        r=Loader.capi.cppcms_capi_session_cookie_first(self.d)
        while r:
            l.append(Cookie(r))
            r=Loader.capi.cppcms_capi_session_cookie_next(self.d)
        self.check()
        return l
    def get_csrf_token(self):
        """Get cppcms CSRF token"""
        r=Loader.capi.cppcms_capi_session_get_csrf_token(self.d)
        self.check()
        return r;
    def get_binary(self,key):
        """Get binary value as bytearray"""
        l=Loader.capi.cppcms_capi_session_get_binary_len(self.d,key)
        res = bytearray(l)
        res_proxy = (c_char * l).from_buffer(res);
        Loader.capi.cppcms_capi_session_get_binary(self.d,key,res_proxy,l)
        self.check()
        return res
    def set_binary(self,key,value):
        """Set binary value as bytearray"""
        if not type(value) is bytearray:
            raise ValueError("value should be bytearray")
        value_proxy = (c_char * len(value)).from_buffer(value);
        Loader.capi.cppcms_capi_session_set_binary(self.d,key,value_proxy,len(value))
        self.check()
    def get(self,key):
        """Get a value for a key"""
        r=Loader.capi.cppcms_capi_session_get(self.d,key)
        self.check()
        return r;
    def set(self,key,value):
        """Set a value for a key"""
        Loader.capi.cppcms_capi_session_set(self.d,key,value)
        self.check()
    def get_session_cookie_name(self):
        """
        Get the name of the cookie that is used to store CppCMS session
        Note: the value of this cookie should be passed to load method
        when session is loaded
        """
        r=Loader.capi.cppcms_capi_session_get_session_cookie_name(self.d)
        self.check()
        return r
    def load(self,cookie=None,django_request=None):
        """
        Load the session directly from cookie value, the name of the cookies
        can be obtained by calling get_session_cookie_name()

        Django users can provide HttpRequest object as django_request and
        the cookies would be retrived automatically
        """
        if cookie!=None:
            Loader.capi.cppcms_capi_session_set_session_cookie(self.d,cookie);
            Loader.capi.cppcms_capi_session_load(self.d)
        elif django_request!=None:
            cookie_name = self.get_session_cookie_name()
            cookie=''
            if cookie_name in django_request.COOKIES:
                cookie = django_request.COOKIES[cookie_name]
            for cookie_name in django_request.COOKIES:
                Loader.capi.cppcms_capi_session_add_cookie_name(self.d,cookie_name)
            Loader.capi.cppcms_capi_session_load(self.d,cookie)
        self.check()
    def save(self,django_response=None):
        """
        Save the session, after the session is saved cookies as provided by cookies()
        method must be set to the response

        Django users can provide HttpResponse object as django_response to set
        the cookies automatically
        """
        Loader.capi.cppcms_capi_session_save(self.d)
        self.check()
        if django_response:
            ck = self.cookies()
            for c in ck:
                key=c.name()
                value=c.value()
                max_age = None
                if(c.max_age_defined()):
                    max_age = c.max_age()
                expires=None
                if(c.expires_defined()):
                    expires=datetime.utcfromtimestamp(c.expires())
                path=None
                if c.path()!='':
                    path=c.path()
                domain=None
                if c.domain()!='':
                    domain=c.domain()
                secure=None
                if c.is_secure():
                    secure=True
                django_response.set_cookie(key, value, max_age, None, path, domain, secure)
                #django_response['Set-Cookie']=c.header_content()
    def __getitem__(self,k):
        if self.is_set(k):
            return self.get(k)
        else:
            raise KeyError("no key " + k + " in session")
    def __setitem__(self,k,v):
        self.set(k,v)
    def __delitem__(self,k):
        if not self.is_set(k):
            raise KeyError("no key " + k + " in session")
        self.erase(k)
    def __contains__(self,k):
        return self.is_set(k)


def __private_test(config):
    def to_hex(a):
        s=''
        for p in a:
            s=s+('\\x%02x' % int(p))
        return s
    state=''
    p=SessionPool(config)
    s=p.session()
    s.load(state)
    s['x']='111'
    s.set('y','222')
    binary=bytearray()
    binary.extend(b'\x01\x00\xFF\x7F')
    s.set_binary('z',binary)
    s.set_exposed('x',1)
    for k in s.keys():
        print ('Got ' + k)
        print ('Value ' + s.get(k))
    s.save()
    for c in s.cookies():
        print (c)
        print (c.value())
        if(c.name()==s.get_session_cookie_name()):
            state = c.value()
    l=None
    s=None
    s=Session(p)
    s.load(state)
    tmp=s.get_binary('z')
    print('Binary expected \\x01\\x00\\xFF\\x7F=' + to_hex(tmp))
    s.set_exposed('x',0)
    s.save()
    print("Use operator:[] " + s['x'])
    print("Is in " + str('x' in s))
    for c in s.cookies():
        print (c)
        print (c.value())
        if(c.name()==s.get_session_cookie_name()):
            state = c.value()
    s=None
    s=Session(p)
    s.load(state)
    del s['y']
    s.clear()
    s.save()
    for c in s.cookies():
        print (c)
        print (c.value())
        if(c.name()==s.get_session_cookie_name()):
            state = c.value()
    print "Test Completed"

if __name__ == "__main__":
    if len(sys.argv) != 2 and len(sys.argv) != 3:
        sys.stderr.write("Usage [ /path/to/libcppcms.so/dll ] config.js\n")
        sys.exit(1)
    if len(sys.argv) == 3:
        Loader.load(sys.argv[1])
        __private_test(sys.argv[2])
    else:
        __private_test(sys.argv[1])


