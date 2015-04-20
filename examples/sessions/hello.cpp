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
    }
    void main(std::string /* unused */)
    {
        int n=0;
        if(session().is_set("n")) {
            n = session().get<int>("n");
            n++;
        }
        session().set<int>("n",n);
        response().set_content_header("text/javascript");
        response().set_header("P3P","CP=\"Make Interent Explorer Happy and Developers really Sad\"");
        response().cache_control("max-age=5");
        response().out() << "document.getElementById('value').innerHTML='<p>" << n << "</p>';" << std::endl;
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
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
