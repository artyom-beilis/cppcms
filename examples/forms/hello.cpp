#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <iostream>
#include "content.h"

using namespace std;
class hello: public cppcms::application {
public:
    hello(cppcms::service &s) :
        cppcms::application(s) 
    {
	    dispatcher().assign("^/?$",&hello::info,this);
    }

    void info()
    {
	    content::message c;
	    if(request().request_method()=="POST") {
		    c.info.load(context());
		    if(c.info.validate()) {
			c.name=c.info.name.value();
			c.sex=c.info.sex.selected_id();
			c.state=c.info.martial.selected_id();
			c.age=c.info.age.value();
			c.info.clear();
		    }
	    }
	    render("message",c);
    }
};

int main(int argc,char ** argv)
{
    try {
        cppcms::service app(argc,argv);
        app.applications_pool().mount(cppcms::applications_factory<hello>());
        app.run();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}


