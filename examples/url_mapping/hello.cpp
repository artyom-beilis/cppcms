#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/applications_pool.h>
#include <iostream>
#include <stdlib.h>


class hello: public cppcms::application {
public:
    hello(cppcms::service &srv) :
        cppcms::application(srv) 
    {
        dispatcher().assign("/number/(\\d+)",&hello::number,this,1);
        dispatcher().assign("/smile",&hello::smile,this);
        dispatcher().assign("",&hello::welcome,this);
    }
    void number(std::string num)
    {
        int no = atoi(num.c_str());
        response().out() << "The number is " << no << "\n";
    }
    void smile()
    {
        response().out() << ":-)";
    }
    void welcome()
    {
        response().out() << 
            "<h1> Wellcome To Page with links </h1>\n"
            "<a href=\"/hello/number/1\">1</a><br>\n"
            "<a href=\"/hello/number/15\">15</a><br>\n"
            "<a href=\"/hello/smile\">:-)</a><br>\n";
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
