from django.http import HttpResponse
from django.conf import settings
import cppcms
#
# Create the session pool - note it is thread safe and should be one per projects
# Provide a path to configuration file
#
pool=cppcms.SessionPool(settings.BASE_DIR + '/config.js')

def bin2hex(val):
    return ''.join('{:02x}'.format(x) for x in val)
def hex2bin(val):
    return bytearray.fromhex(val)
    

def home(request):
    session=pool.session()
    session.load(django_request=request)
    i = 0
    output = []
    while True:
        i=i+1
        id = "_" + str(i)
        op_id = 'op' + id
        key_id = 'key' + id
        value_id = 'value' + id
        print 'Checking ', op_id
        if not op_id in request.GET:
            break
        op = request.GET[op_id]
        if key_id in request.GET:
            key = request.GET[key_id]
        if value_id in request.GET:
            value = request.GET[value_id]
        result = 'ok'
        if op=="is_set":
            result = 'yes' if key in session else 'no'
        elif op == "erase":
            del session[key]
        elif op == "clear":  
            session.clear();
        elif op == "is_exposed":  
            result = 'yes' if session.get_exposed(key) else "no";
        elif op == "expose":  
            session.set_exposed(key,int(value));
        elif op == "get":  
            result = session[key];
        elif op == "set":  
            session[key]=value;
        elif op == "get_binary":  
            result = bin2hex(session.get_binary(key));
        elif op == "set_binary":  
            session.set_binary(key,hex2bin(value));
        elif op == "get_age":  
            result = str(session.get_age())
        elif op == "set_age":  
            session.set_age(value)
            print 'SET AGE DONE', value
        elif op == "default_age":  
            session.default_age();
        elif op == "get_expiration":  
            result = str(session.get_expiration())
        elif op == "set_expiration":  
            session.set_expiration(int(value))
        elif op == "default_expiration":  
            session.default_expiration();
        elif op == "get_on_server":  
            result = 'yes' if session.get_on_server() else "no"
        elif op == "set_on_server":  
            session.set_on_server(int(value))
        elif op == "reset_session":  
            session.reset_session();
        elif op == "csrf_token":  
            result = "t="  + session.csrf_token;
        elif op == "keys":  
            ks=[]
            for key in session.keys:
                ks.append('[' + key + ']')
            result = ','.join(ks)
        else:
            result = "invalid op=" + op;
        msg = str(i) + ':' + result + ';'
        print 'Res ' + msg
        output.append(msg);
    response = HttpResponse()
    session.save(django_response=response)
    response.write(''.join(output));
    return response
