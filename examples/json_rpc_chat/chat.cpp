///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
        bind("post",cppcms::rpc::json_method(&chat::post,this),notification_role);
        bind("get",cppcms::rpc::json_method(&chat::get,this),method_role);
        last_wake_ = time(0);
        on_timer(booster::system::error_code());
    }
    void post(std::string const &author,std::string const &message)
    {
        cppcms::json::value obj;
        obj["author"]=author;
        obj["message"]=message;
        messages_.push_back(obj);
        last_wake_ = time(0);
        broadcast(messages_.size()-1);
    }

    void on_timer(booster::system::error_code const &e)
    {
        if(e) return; // cancelation
        if(last_wake_ - time(0) > 10) {
            broadcast(messages_.size());
            last_wake_ = time(0);
        }
        timer_.expires_from_now(booster::ptime::seconds(1));
        timer_.async_wait(boost::bind(&chat::on_timer,booster::intrusive_ptr<chat>(this),_1));
        std::cout << "Status: \n"
            << "Waiters: " << waiters_.size() << '\n'
            << "Messages:" << messages_.size() <<'\n'
            << "[";
        for(size_t i=0;i<messages_.size();i++)
            std::cout << messages_[i] << std::endl;
        std::cout <<"]" << std::endl;
    }

    void get(unsigned from)
    {
        if(from < messages_.size()) {
            return_result(make_response(from));
            return;
        }
        else if(from == messages_.size()) {
            booster::shared_ptr<cppcms::rpc::json_call> call=release_call();
            waiters_.insert(call);
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
    void remove_context(booster::shared_ptr<cppcms::rpc::json_call> call)
    {
        waiters_.erase(call);
    }
    void broadcast(size_t from)
    {
        for(waiters_type::iterator waiter=waiters_.begin();waiter!=waiters_.end();++waiter) {
            booster::shared_ptr<cppcms::rpc::json_call> call = *waiter;
            call->return_result(make_response(from));
        }
        waiters_.clear();
    }
    cppcms::json::value make_response(size_t n)
    {
        cppcms::json::value v;

        v=cppcms::json::array();
        cppcms::json::array &ar  = v.array();

        ar.reserve(messages_.size() - n);
        for(size_t i=n;i<messages_.size();i++) {
            ar.push_back(messages_[i]);
        }
        std::cout << "Response to client:" << v << std::endl;
        return v;
    }
private:
    std::vector<cppcms::json::value> messages_;
    typedef std::set<booster::shared_ptr<cppcms::rpc::json_call> > waiters_type;
    waiters_type waiters_;
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
