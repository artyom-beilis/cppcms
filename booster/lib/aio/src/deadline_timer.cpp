//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/aio/deadline_timer.h>
#include <booster/aio/io_service.h>

namespace booster {
namespace aio {

struct deadline_timer::data {} ;

deadline_timer::deadline_timer() : 
	srv_(0),
	deadline_(ptime::now()),
	event_id_(-1)
{
}

deadline_timer::deadline_timer(io_service &srv) : 
	srv_(&srv),
	deadline_(ptime::now()),
	event_id_(-1)
{
}

deadline_timer::~deadline_timer()
{
}

io_service &deadline_timer::get_io_service()
{
	if(!srv_)
		throw system::system_error(system::error_code(aio_error::no_service_provided,aio_error_cat));
	return *srv_;
}

void deadline_timer::reset_io_service()
{
	srv_ = 0;
}

void deadline_timer::set_io_service(io_service &srv)
{
	reset_io_service();
	srv_ = &srv;
}

void deadline_timer::expires_from_now(ptime t)
{
	deadline_ = ptime::now() + t;
}

ptime deadline_timer::expires_from_now()
{
	return deadline_ - ptime::now();
}

void deadline_timer::expires_at(ptime t)
{
	deadline_ = t;
}

ptime deadline_timer::expires_at()
{
	return deadline_;
}

void deadline_timer::wait()
{
	ptime diff = expires_from_now();
	if(diff <= ptime::zero)
		return;
	ptime::sleep(diff);
}

struct deadline_timer::waiter {
	event_handler h;
	deadline_timer *self;
	void operator()(system::error_code const &e) const
	{
		self->event_id_ = -1;
		h(e);
	}
};

void deadline_timer::async_wait(event_handler const &h)
{
	waiter wt = { h, this };
	event_id_ = get_io_service().set_timer_event(deadline_,wt);
}

void deadline_timer::cancel()
{
	if(event_id_!=-1) {
		int tmp_id = event_id_;
		event_id_ = -1;
		get_io_service().cancel_timer_event(tmp_id);
	}
}


} // aio
} // booster

