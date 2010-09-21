#include <cppcms/service.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/cache_interface.h>
#include <iostream>
#include <sstream>
#include "content.h"

class hello: public cppcms::application {
public:
    hello(cppcms::service &s) :
        cppcms::application(s) 
    {
    }
    void main(std::string /*unused*/)
    {
        content::message c;
        c.arg=0;
        c.fact=1;
        if(request().request_method()=="POST") {
            c.info.load(context());
            if(c.info.validate()) {
                c.arg=c.info.arg.value(); 
            c.info.clear();
            }
            else { // No cache should be used
               render("message",c);
            return;
            }
        }

        std::ostringstream key;
        key << "factorial_" << c.arg;
        if(cache().fetch_page(key.str()))
            return;
        long long int f=1;
        for(int i=1;i<=c.arg;i++) {
            f*=i;
        }
        c.fact=f;
        render("message",c);
        cache().store_page(key.str(),3600);
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
