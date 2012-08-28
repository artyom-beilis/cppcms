//
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>
//
//  See accompanying file COPYING.TXT file for licensing details.
//
#include "server_sent_events.h"

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_context.h>
#include <booster/aio/deadline_timer.h>
#include <booster/system_error.h>

#include <sstream>

class ticker : public cppcms::application {
public:
    ticker(cppcms::service &srv) : 
        cppcms::application(srv),
        tm_(srv.get_io_service()),
        price_(1.0)
    {
        stream_ = sse::state_stream::create(srv.get_io_service());
        wait();
    }

    void wait()
    {
        tm_.expires_from_now(booster::ptime::from_number(double(rand())/RAND_MAX + 0.01));
        tm_.async_wait([=](booster::system::error_code const &e){
            if(!e) {
                on_timer();
                wait();
            }
        });
    }

    void on_timer()
    {
        price_ += double(rand()) / RAND_MAX * 2.0 - 1;
        if(price_ <= 0.01)
            price_ = 0.01;
        std::ostringstream ss;
        ss << price_;
        stream_->update(ss.str());
    }
    void main(std::string /*url*/)
    {
        stream_->accept(release_context());
    }
    
private:
    booster::shared_ptr<sse::state_stream> stream_;
    booster::aio::deadline_timer tm_;
    double price_;
};


int main(int argc,char **argv)
{
    try {
        cppcms::service service(argc,argv);
        booster::intrusive_ptr<ticker> c=new ticker(service);
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


