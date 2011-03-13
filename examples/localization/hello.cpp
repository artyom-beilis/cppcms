#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/applications_pool.h>
#include <iostream>
#include "content.h"


class hello: public cppcms::application {
public:
    hello(cppcms::service &srv) :
        cppcms::application(srv) 
    {
        dispatcher().assign("^/(en_US|he_IL)/?$",&hello::say_hello,this,1);
        dispatcher().assign("^(.*)$",&hello::redirect,this);
    }
    void redirect()
    {
        response().set_redirect_header(request().script_name() + "/en_US");
    }
    void say_hello(std::string lang)
    {
        context().locale(lang + ".UTF-8");
        content::message c;
        c.message=cppcms::locale::translate("Hello World");
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
        std::cerr<<e.what()<<std::endl;
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
