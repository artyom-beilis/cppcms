//
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>
//
//  See accompanying file COPYING.TXT file for licensing details.
//
#include <cppcms/application.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <cppcms/http_context.h>
#include <booster/intrusive_ptr.h>
#include "server_sent_events.h"

#include <set>

class chat : public cppcms::application {
public:
    chat(cppcms::service &srv) : 
        cppcms::application(srv)
    {
        queue_ = sse::bounded_event_queue::create(srv.get_io_service(),1024);
        queue_->enable_keep_alive(10);
        dispatcher().assign("/post",&chat::post,this);
        dispatcher().assign("/get",&chat::get,this);
        dispatcher().assign(".*",&chat::redirect,this);
    }
    void redirect()
    {
        response().set_redirect_header("/the_chat.html");
    }
    void post()
    {
        if(request().request_method()=="POST") {
            std::string message = request().post("message");
            queue_->enqueue(message);
        }
    }
    void get()
    {
        queue_->accept(release_context());
    }
private:
    booster::shared_ptr<sse::bounded_event_queue> queue_;
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

