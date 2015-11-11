from django.http import HttpResponse
from django.conf import settings
import cppcms

cppcms.Loader.load(settings.CPPCMS_LIB)
pool=cppcms.SessionPool(settings.CPPCMS_CONFIG)

def home(request):
	s=pool.session()
	s.load(django_request=request)
	v='Nothing'
	if not 'x' in s:
		s['x']='1'
	else:
		v=s['x']
		s['x']=str(int(v)+1)

	response = HttpResponse()
	s.save(django_response=response)
	response['Set-Cookie']='test=xxx'
	response.write('<html><body><h1>Cookies</h1>')
	response.write('<p>value=%s</p>' % v)
	for key,value in request.COOKIES.iteritems():
		response.write('%s=%s<br>\n' % (key,value))
	response.write('</body></html>')
	return response
