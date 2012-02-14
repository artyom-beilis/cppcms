#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <iostream>

#include "content.h"

class myapp  : public cppcms::application {
public:
    myapp(cppcms::service &s) :
       cppcms::application(s)
    {
        dispatcher().assign("",&myapp::intro,this);
        mapper().assign("");

        dispatcher().assign("/news",&myapp::news,this);
        mapper().assign("news","/news");

        dispatcher().assign("/page",&myapp::page,this);
        mapper().assign("page","/page");

        mapper().root("/myapp");
    }
    void ini(content::master &c)
    {
        c.title = "My Web Site";
    }
    void intro()
    {
        content::master c;
        ini(c);
        render("intro",c);
    }
    void page()
    {
        content::page c;
        ini(c);
        c.page_title = "About";
        c.page_content = "<p>A page about this web site</p>";
        render("page",c);
    }
    void news()
    {
        content::news c;
        ini(c);
        c.news_list.push_back("This is the latest message!");
        c.news_list.push_back("This is the next message.");
        c.news_list.push_back("This is the last message!");
        render("news",c);
    }
};



int main(int argc,char ** argv)
{
    try {
        cppcms::service srv(argc,argv);
        srv.applications_pool().mount(cppcms::applications_factory<myapp>());
        srv.run();
    }
    catch(std::exception const &e) {
        std::cerr<<e.what()<<std::endl;
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

