#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/applications_pool.h>
#include <iostream>
#include <stdlib.h>


class hello: public cppcms::application {
public:
    hello(cppcms::service &srv) :
        cppcms::application(srv) 
    {
        dispatcher().assign("/number/(\\d+)",&hello::number,this,1);
        mapper().assign("number","/number/{1}");
        
        dispatcher().assign("/smile",&hello::smile,this);
        mapper().assign("smile","/smile");

        dispatcher().assign("",&hello::welcome,this);
        mapper().assign("");

        mapper().root("/hello");
    }
    void number(std::string num)
    {
        int no = atoi(num.c_str());
        response().out() << "The number is " << no << "<br/>\n";
        response().out() << "<a href='" << url("/") << "'>Go back</a>";
    }
    void smile()
    {
        response().out() << ":-) <br/>\n";
        response().out() << "<a href='" << url("/") << "'>Go back</a>";
    }
    void welcome()
    {
        response().out() << 
            "<h1> Wellcome To Page with links </h1>\n"
            "<a href='" << url("/number",1)  << "'>1</a><br>\n"
            "<a href='" << url("/number",15) << "'>15</a><br>\n"
            "<a href='" << url("/smile") << "' >:-)</a><br>\n";
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
