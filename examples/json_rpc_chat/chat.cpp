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
#include <cppcms/rpc_json.h>
#include <cppcms/json.h>
#include <booster/intrusive_ptr.h>
#include <booster/aio/deadline_timer.h>
#include <booster/system_error.h>

#include <set>

#include <boost/bind.hpp>
using boost::bind;


class chat : public cppcms::rpc::json_rpc_server {
public:
    chat(cppcms::service &srv) : 
        cppcms::rpc::json_rpc_server(srv),
        timer_(srv.get_io_service())
    {
        // Our main methods
        bind("post",cppcms::rpc::json_method(&chat::post,this),notification_role);
        bind("get",cppcms::rpc::json_method(&chat::get,this),method_role);
        
        // Add timeouts to the system
        last_wake_ = time(0);
        on_timer(booster::system::error_code());
    }

    // Handle new message call
    void post(std::string const &author,std::string const &message)
    {
        cppcms::json::value obj;
        obj["author"]=author;
        obj["message"]=message;
        messages_.push_back(obj);
        broadcast(messages_.size()-1);
    }

    void on_timer(booster::system::error_code const &e)
    {
        if(e) return; // cancelation

        // check idle connections for more then 10 seconds
        if(time(0) - last_wake_ > 10) {
            broadcast(messages_.size());
        }
        // restart timer
        timer_.expires_from_now(booster::ptime::seconds(1));
        timer_.async_wait(boost::bind(&chat::on_timer,booster::intrusive_ptr<chat>(this),_1));
    }

    // Handle request
    void get(unsigned from)
    {
        if(from < messages_.size()) {
            // not long polling - return result now
            return_result(make_response(from));
            return;
        }
        else if(from == messages_.size()) {
            // Can't answer now

            // Add long polling request to the list

            booster::shared_ptr<cppcms::rpc::json_call> call=release_call();
            waiters_.insert(call);

            // set disconnect callback
            call->context().async_on_peer_reset(
                boost::bind(
                    &chat::remove_context,
                    booster::intrusive_ptr<chat>(this),
                    call));
        }
        else {
            return_error("Invalid position");
        }
    }

    // handle client disconnect
    void remove_context(booster::shared_ptr<cppcms::rpc::json_call> call)
    {
        waiters_.erase(call);
    }

    void broadcast(size_t from)
    {
        // update timeout
        last_wake_ = time(0);
        // Prepare response
        cppcms::json::value response = make_response(from);
        // Send it to everybody
        for(waiters_type::iterator waiter=waiters_.begin();waiter!=waiters_.end();++waiter) {
            booster::shared_ptr<cppcms::rpc::json_call> call = *waiter;
            call->return_result(response);
        }
        waiters_.clear();
    }

    // Prepare response to the client
    cppcms::json::value make_response(size_t n)
    {
        cppcms::json::value v;

        // Small optimization
        v=cppcms::json::array();
        cppcms::json::array &ar  = v.array();
        ar.reserve(messages_.size() - n);

        // prepare all messages
        for(size_t i=n;i<messages_.size();i++) {
            ar.push_back(messages_[i]);
        }
        return v;
    }
private:

    // message store
    std::vector<cppcms::json::value> messages_;

    // long poll requests
    typedef std::set<booster::shared_ptr<cppcms::rpc::json_call> > waiters_type;
    waiters_type waiters_;

    // timer for resetting idle requests
    booster::aio::deadline_timer timer_;
    time_t last_wake_;
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
