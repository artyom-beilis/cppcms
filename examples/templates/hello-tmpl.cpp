#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <iostream>

#include "content.h"

class my_hello_world : public cppcms::application {
public:
    my_hello_world(cppcms::service &s) :
       cppcms::application(s)
    {
    }
    virtual void main(std::string /*url*/)
    {
        content::message c;
        c.text=">>>Hello<<<";
        render("message",c);
    }
};



int main(int argc,char ** argv)
{
    try {
        cppcms::service srv(argc,argv);
        srv.applications_pool().mount(cppcms::applications_factory<my_hello_world>());
        srv.run();
    }
    catch(std::exception const &e) {
        std::cerr<<e.what()<<std::endl;
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

