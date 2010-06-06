#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/session_interface.h>
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
			    session()["name"]=c.info.name.value();
			    session()["sex"]=c.info.sex.selected_id();
			    session()["state"]=c.info.martial.selected_id();
			    session().set("age",c.info.age.value());
			    c.info.clear();
		    }
	    }

	    if(session().is_set("name")) {
		c.name=session()["name"];
		if(session()["sex"]=="m") {
			c.who="Mr";
		}
		else {
			if(session()["state"]=="s") {
				c.who="Miss";
			}
			else {
				c.who="Mrs";
			}
		}
		c.age=session().get<double>("age");
	    }
	    else {
		c.name="Visitor";
		c.age=-1;
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
