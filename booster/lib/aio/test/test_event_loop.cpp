//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "aio_test.h"
#include <booster/aio/io_service.h>
#include <booster/posix_time.h>

bool done;
bool timer_done;

using booster::ptime;

struct sock1 {
	sock1(booster::aio::native_type f,booster::aio::io_service *s) :
		fd(f),
		srv(s),
		counter(0)
	{
	}
	booster::aio::native_type fd;
	booster::aio::io_service *srv;
	int counter;
	void async_run()
	{
		srv->set_io_event(fd,booster::aio::io_service::in,*this);
	}
	void operator()(booster::system::error_code const &e)
	{
		TEST(!e);
		char c=-1;
		int n = ::recv(fd,&c,1,0);
		if(counter == 10) {
			TEST(n==0);
			srv->stop();
			done=true;
		}
		else  {
			TEST(c==counter);
			counter++;
			async_run();
		}

	}
};

struct sock2 {
	sock2(booster::aio::native_type f,booster::aio::io_service *s) :
		fd(f),
		srv(s),
		counter(0)
	{
	}
	booster::aio::native_type fd;
	booster::aio::io_service *srv;
	int counter;
	void async_run()
	{
		srv->set_io_event(fd,booster::aio::io_service::out,*this);
	}
	void operator()(booster::system::error_code const &e)
	{
		TEST(!e);
		char c=counter++;
		int n = ::send(fd,&c,1,0);
		TEST(n==1);
		if(counter == 10) {
			#ifdef SHUT_RDWR
			shutdown(fd,SHUT_RDWR);
			#else
			shutdown(fd,SD_BOTH);
			#endif
		}
		else  {
			async_run();
		}

	}
};


struct timer {
	booster::aio::io_service *srv;
	booster::ptime t;
	int event_id;
	bool cancel;
	void operator()(booster::system::error_code const &e)
	{
		if(cancel) {
			TEST(e==booster::system::error_code(booster::aio::aio_error::canceled,booster::aio::aio_error_cat));
			srv->stop();
			timer_done = true;
		}
		else {
			TEST(!e);
			TEST(booster::ptime::now() >= t);
			cancel=true;
			event_id = srv->set_timer_event(booster::ptime::now() + ptime::seconds(10),*this);
			canceler c = {event_id,srv};
			srv->set_timer_event(booster::ptime::now() + ptime::milliseconds(100),c);
		}

	}
	struct canceler {
		int event_id;
		booster::aio::io_service *srv;
		void operator()(booster::system::error_code const &e) const
		{
			TEST(!e);
			srv->cancel_timer_event(event_id);
		}
	};
};

bool cancel_called = 0;
bool post_executed = false;

struct cancel_handler {
	booster::aio::io_service *srv;
	void operator()() const
	{
		post_executed=true;
		srv->stop();
	}
	void operator()(booster::system::error_code const &e) const
	{
		TEST(e==booster::system::error_code(booster::aio::aio_error::canceled,booster::aio::aio_error_cat));
		cancel_called ++;
		srv->post(*this);
	}
};

struct canceler {
	booster::aio::native_type fd;
	booster::aio::io_service *srv;
	void operator()(booster::system::error_code const &e)
	{
		TEST(!e);
		srv->cancel_io_events(fd);
	}
};


booster::aio::io_service *the_service;
booster::aio::native_type the_socket;

bool got_error_called = false;

void got_error(booster::system::error_code const &e)
{
	TEST(e);
	got_error_called=true;
	the_service->stop();
}

void run_closer()
{
	booster::ptime::millisleep(100);
	#ifdef BOOSTER_WIN32
	closesocket(the_socket);
	#else
	close(the_socket);
	#endif
}

void run_stopper(booster::system::error_code const &/*e*/)
{
	the_service->stop();
}

void post_executor()
{
	post_executed = true;
}
void poster()
{
	booster::ptime::millisleep(50);
	the_service->post(post_executor);
	booster::ptime::millisleep(50);
	the_service->stop();
}

int main()
{
	try {
		std::cout << "Testing IO events" << std::endl;
		booster::aio::io_service srv;
		{
			booster::aio::native_type fds[2];
			pair(fds);
			sock1 s1(fds[0],&srv);
			sock2 s2(fds[1],&srv);
			s1.async_run();
			s2.async_run();
			srv.run();
			TEST(done);
		}
		srv.reset();
		{
			std::cout << "Testing timer events" << std::endl;
			timer tt = { &srv, booster::ptime::now() + ptime::milliseconds(100), 0, false };
			srv.set_timer_event(tt.t,tt);
			srv.run();
			TEST(timer_done);
		}
		srv.reset();
		{
			std::cout << "Testing IO events cancelation" << std::endl;
			booster::aio::native_type fds[2];
			pair(fds);
			cancel_handler ch = { &srv };
			srv.set_io_event(fds[0],booster::aio::io_service::in,ch);
			canceler c = { fds[0], &srv };
			srv.set_timer_event(booster::ptime::now() + ptime::milliseconds(100),c);
			srv.run();
			TEST(cancel_called==1);
			std::cout << "Testing post()" << std::endl;
			TEST(post_executed);
		}
		the_service = &srv;
		srv.reset();
		{
			std::cout << "Testing adding invalid socket id" << std::endl;
			srv.set_io_event(booster::aio::invalid_socket,booster::aio::io_service::in,got_error);
			got_error_called = false;
			srv.run();
			TEST(got_error_called);
		}
		srv.reset();
		{
			std::cout << "Testing event from other thread" << std::endl;
			post_executed = false;
			booster::thread p(poster);
			srv.run();
			p.join();
			TEST(post_executed);
		}
	}
	catch(std::exception const &e)
	{
		std::cout<<"Failed:"<< e.what() << std::endl;
		return 1;
	}
	if(return_code == 0)
		std::cout << "Ok" << std::endl;
	return return_code;
}
