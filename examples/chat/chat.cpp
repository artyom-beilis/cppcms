///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/application.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <cppcms/http_context.h>
#include <booster/intrusive_ptr.h>

#include <set>

#ifdef USE_STD_TR1_BIND
  #include <tr1/functional>
  using std::tr1::bind;
#elif defined USE_STD_BIND
  #include <functional>
  using std::bind;
#else
  #include <boost/bind.hpp>
  using boost::bind;
#endif


class chat : public cppcms::application {
public:
    chat(cppcms::service &srv) : cppcms::application(srv)
    {
        dispatcher().assign("/post",&chat::post,this);
        dispatcher().assign("/get/(\\d+)",&chat::get,this,1);
        dispatcher().assign(".*",&chat::redirect,this);
    }
    void redirect()
    {
        response().set_redirect_header("/the_chat.html");
    }
    void post()
    {
        if(request().request_method()=="POST") {
            if(request().post().find("message")!=request().post().end()) {
                messages_.push_back(request().post().find("message")->second);
                broadcast();
            }
        }
        
        release_context()->async_complete_response();
    }
    void get(std::string no)
    {
        unsigned pos=atoi(no.c_str());
        if(pos < messages_.size()) {
            response().set_plain_text_header();
            response().out()<<messages_[pos];
            release_context()->async_complete_response();
        }
        else if(pos == messages_.size()) {
            booster::shared_ptr<cppcms::http::context> context=release_context();
            waiters_.insert(context);
            context->async_on_peer_reset(
                bind(
                    &chat::remove_context,
                    booster::intrusive_ptr<chat>(this),
                    context));
        }
        else {
            response().status(404);
            release_context()->async_complete_response();
        }
    }
    void remove_context(booster::shared_ptr<cppcms::http::context> context)
    {
        waiters_.erase(context);
    }
    void broadcast()
    {
        for(waiters_type::iterator waiter=waiters_.begin();waiter!=waiters_.end();++waiter) {
            (*waiter)->response().set_plain_text_header();
            (*waiter)->response().out() << messages_.back();
            (*waiter)->async_complete_response();
        }
        waiters_.clear();
    }
private:
    std::vector<std::string> messages_;
    typedef std::set<booster::shared_ptr<cppcms::http::context> > waiters_type;
    waiters_type waiters_;
};


int main(int argc,char **argv)
{
    try {
        cppcms::service service(argc,argv);
        booster::intrusive_ptr<chat> c=new chat(service);
        service.applications_pool().mount(c);
        service.run();
    }
    catch(std::exception const &e) {
        std::cerr<<"Catched exception: "<<e.what()<<std::endl;
        return 1;
    }
    return 0;
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
