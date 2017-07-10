from django.conf.urls import patterns, include, url

from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    # Examples:
    # url(r'^$', 'cppcmstest.views.home', name='home'),
    url(r'^tester/', include('tester.urls')),

    url(r'^admin/', include(admin.site.urls)),
)
