#!/usr/bin/env python
# coding=UTF-8
# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


from httpclient import Cookie, Session
import sys
import time

def test(X):
    if not X:
        raise Exception('Test failed')

def parse_info(resp):
    print 'Got info %s' % resp
    dic={}
    for pair in resp.split('\n'):
        dic[pair.split('=')[0]] = pair.split('=')[1]
    #print "Parsed as ", dic
    return dic

if len(sys.argv) < 3:
    print "Usage: (server|client|both) (fixed|renew|browser)"
    sys.exit(1)

def test_method_is(dic,method):
    v = dic['expiration']
    if method=='fixed':
        test(v=='0')
    elif method == 'renew':
        test(v=='1')
    else: # browser
        test(v=='2')

def general_tests(method):
    print "------------------- General Tests --------------------"
    session = Session()
    session.transmit('/new')
    cookie = session.received['sc']
    if method=='fixed' or method=='renew':
        test(cookie.max_age == '5')
    else:
        test(not cookie.max_age)
   
    test(cookie.domain == 'foo.bar')
    test(cookie.path == '/foo')

    time.sleep(2)

    session.transmit('/update')
    cookie = session.received['sc']
    
    if method=='fixed':
        test(int(cookie.max_age) < 5)
    elif method=='renew':
        test(int(cookie.max_age) == 5)
    
    test(session.transmit('/clear') == 'clear')
    test(session.received['sc'].max_age == '0')
    test(parse_info(session.transmit('/info'))['is_set_x']=='0')

    session.transmit('/new_short')
    if method!='browser':
        test(session.received['sc'].max_age == '1')
    time.sleep(2)
    test(session.transmit('/is_expired') == 'expired')

    print "new"
    session.transmit('/new')
    print "expose"
    session.transmit('/expose')
    test(session.received['sc_x'].value=='1')
    print "update"
    session.transmit('/update')
    test(session.received['sc_x'].value=='2')
    print "clear"
    session.transmit('/clear')
    test(session.received['sc_x'].max_age=='0')
    session.transmit('/new')
    test(parse_info(session.transmit('/info'))['is_exposed_x']=='0')
    session.transmit('/expose')
    test(session.received['sc_x'].value=='1')
    test(parse_info(session.transmit('/info'))['is_exposed_x']=='1')
    session.transmit('/unexpose')
    test(session.received['sc_x'].max_age=='0')
    test(parse_info(session.transmit('/info'))['is_exposed_x']=='0')
    test('sc' in session.state)
    session.transmit('/clear')
    test(session.received['sc'].max_age=='0')
    test(not 'sc_x' in session.received)

    session.transmit('/new')
    dic = parse_info(session.transmit('/info'))
    test(dic['is_set_x']=='1')
    test(dic['age']=='5')
    test_method_is(dic,method)
    session.transmit('/fixed')
    test(session.received['sc'].max_age=='5')
    test_method_is(parse_info(session.transmit('/info')),'fixed')
    time.sleep(2);
    test_method_is(parse_info(session.transmit('/info')),'fixed')
    test(not 'sc' in session.received)
    session.transmit('/renew')
    test(session.received['sc'].max_age=='5')
    test_method_is(parse_info(session.transmit('/info')),'renew')
    time.sleep(2);
    test_method_is(parse_info(session.transmit('/info')),'renew')
    test(session.received['sc'].max_age=='5')
    session.transmit('/browser')
    test(not session.received['sc'].max_age)
    test_method_is(parse_info(session.transmit('/info')),'browser')
    session.transmit('/clear')
    session.transmit('/new')
    test(parse_info(session.transmit('/info'))['age']=='5')
    session.transmit('/new_short')
    test(parse_info(session.transmit('/info'))['age']=='1')
    session.transmit('/clear')
    test(session.transmit('/api')=='ok')



def reset_session(x):
    print "------------------- Reset Session --------------------"
    session=Session()
    session.transmit('/new')
    prev_cookie = session.state['sc'].value
    session.transmit('/reset')
    new_cookie = session.state['sc'].value
    test(prev_cookie != new_cookie)
    test(prev_cookie[0]=='I')
    test(new_cookie[0]=='I')
    test(parse_info(session.transmit('/info'))['is_set_x']=='1')
def no_replay(x):
    session=Session()
    session.transmit('/new')
    old_state = session.state
    session.transmit('/clear')
    session.state = old_state
    test(parse_info(session.transmit('/info'))['is_set_x']=='0')
def force_server(x):
    print "------------------- Force Server --------------------"
    session=Session()
    session.transmit('/new')
    session.transmit('/update')
    test(session.state['sc'].value[0]=='C')
    test(parse_info(session.transmit('/info'))['on_server']=='0')
    session.transmit('/on_server')
    test(session.state['sc'].value[0]=='I')
    test(parse_info(session.transmit('/info'))['on_server']=='1')
    session.transmit('/update')
    test(session.state['sc'].value[0]=='I')
    test(parse_info(session.transmit('/info'))['on_server']=='1')
    session.transmit('/not_on_server')
    test(session.state['sc'].value[0]=='C')
    test(parse_info(session.transmit('/info'))['on_server']=='0')
    session.transmit('/update')
    test(session.state['sc'].value[0]=='C')
    test(parse_info(session.transmit('/info'))['on_server']=='0')
def size_tests(x):
    print "------------------- Force Server --------------------"
    session=Session()
    session.transmit("/huge")
    test(session.state['sc'].value[0]=='I')
    session.transmit("/small")
    test(session.state['sc'].value[0]=='C')
    session=Session()
    session.transmit("/new")
    test(session.state['sc'].value[0]=='C')
    session.transmit("/huge")
    test(session.state['sc'].value[0]=='I')
    session.transmit("/on_server")
    test(session.state['sc'].value[0]=='I')
    session.transmit("/not_on_server")
    test(session.state['sc'].value[0]=='I')
    session.transmit("/small")
    test(session.state['sc'].value[0]=='C')

method = sys.argv[2]


if sys.argv[1] == 'server':
    general_tests(method)
    reset_session(method)
    no_replay(method)
elif sys.argv[1] == 'client':
    general_tests(method)
elif sys.argv[1] == 'both':
    general_tests(method)
    size_tests(method)
    force_server(method)



