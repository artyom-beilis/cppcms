#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/applications_pool.h>
#include <iostream>
#include <stdlib.h>


class numbers : public cppcms::application {
public:
    numbers(cppcms::service &srv) : cppcms::application(srv)
    {
        dispatcher().assign("",&numbers::all,this);
        mapper().assign("");

        dispatcher().assign("/odd",&numbers::odd,this);
        mapper().assign("odd","/odd");

        dispatcher().assign("/even",&numbers::even,this);
        mapper().assign("even","/even");

        dispatcher().assign("/prime",&numbers::prime,this);
        mapper().assign("prime","/prime");
    }

    void all()
    {
        response().out() 
            << "<a href='" << url("/")       << "'>Top</a><br>"
            << "<a href='" << url("/letters")<< "'>Letters</a><br>"
            << "<a href='" << url(".")       << "'>All Numbers</a><br>"
            << "<a href='" << url("odd")     << "'>Odd Numbers</a><br>"
            << "<a href='" << url("even")    << "'>Even Numbers</a><br>"
            << "<a href='" << url("prime")   << "'>Prime Numbers</a><br>"
            << "1,2,3,4,5,6,7,8,9,10,...";
    }

    void prime()
    {
        response().out() << "2,3,5,7,...";
    }
    void odd()
    {
        response().out() << "1,3,5,7,9,...";
    }
    void even()
    {
        response().out() << "2,4,6,8,10,...";
    }
};

class letters : public cppcms::application {
public:
    letters(cppcms::service &srv) : cppcms::application(srv)
    {
        dispatcher().assign("",&letters::all,this);
        mapper().assign("");

        dispatcher().assign("/capital",&letters::capital,this);
        mapper().assign("capital","/capital");

        dispatcher().assign("/small",&letters::small,this);
        mapper().assign("small","/small");

    }

    void all()
    {
        response().out() 
            << "<a href='" << url("/")       << "'>Top</a><br>"
            << "<a href='" << url("/numbers")<< "'>Numbers</a><br>"
            << "<a href='" << url(".")       << "'>All Letters</a><br>"
            << "<a href='" << url("capital") << "'>Capital Letters</a><br>"
            << "<a href='" << url("small")   << "'>Small Letters</a><br>"
            << "Aa, Bb, Cc, Dd,...";
    }

    void capital()
    {
        response().out() << "A,B,C,D,...";
    }
    void small()
    {
        response().out() << "a,b,c,d,...";
    }
};



class myapp: public cppcms::application {
public:
    myapp(cppcms::service &srv) :
        cppcms::application(srv) 
    {
        attach( new numbers(srv),
                "numbers", "/numbers{1}", // mapping
                "/numbers(/(.*))?", 1);   // dispatching
        attach( new letters(srv),
                "letters", "/letters{1}", // mapping
                "/letters(/(.*))?", 1);   // dispatching

        dispatcher().assign("",&myapp::describe,this);
        mapper().assign(""); // default URL
        
        mapper().root("/myapp");
    }
    void describe()
    {
        response().out() 
            << "<a href='" << url("/numbers")<< "'>Numbers</a><br>"
            << "<a href='" << url("/letters")<< "'>Letters</a><br>"
            << "<a href='" << url("/numbers/odd")<< "'>Odd Numbers</a><br>";
    }
};

int main(int argc,char ** argv)
{
    try {
        cppcms::service app(argc,argv);
        app.applications_pool().mount(cppcms::applications_factory<myapp>());
        app.run();
    }
    catch(std::exception const &e) {
        std::cerr<<e.what()<<std::endl;
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
