from django.http import HttpResponse
from django.conf import settings
import cppcms
#
# Create the session pool - note it is thread safe and should be one per projects
# Provide a path to configuration file
#
pool=cppcms.SessionPool(settings.BASE_DIR + '/mysite/cppcms-config.js')

def home(request):
	# Get session object
	s=pool.session()
	# Load the data from request
	s.load(django_request=request)

	# Access session key-values as dictionary
	v='0'
	if 'x' in s:
		v= s['x']
	s['x']=str(int(v)+1)

	response = HttpResponse()

	# Save the session
	s.save(django_response=response)
	response.write('x is %s' % v);
	return response
