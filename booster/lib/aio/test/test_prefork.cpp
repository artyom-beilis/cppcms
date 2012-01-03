//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/thread.h>
#include "test.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <iostream>
#include <errno.h>

#include <booster/aio/io_service.h>
#include <booster/aio/socket.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/aio_category.h>
#include <booster/aio/buffer.h>
#include <booster/aio/deadline_timer.h>
#include <booster/posix_time.h>
#include <booster/system_error.h>
namespace io = booster::aio;
namespace sys = booster::system;

io::io_service *the_service;
io::acceptor *the_socket;
io::stream_socket *accepted_socket;

void socket_canceler(sys::error_code const &e)
{
	TEST(!e);
	the_socket->cancel();
}

bool called=false;

void socket_accept_failer(sys::error_code const &e)
{
	TEST(e);
	TEST(e.value()==io::aio_error::canceled);
	the_service->stop();
	called=true;
}

void thread_socket_canceler()
{
	booster::ptime::millisleep(100);
	the_socket->cancel();
}

void thread_socket_connector()
{
	try {
		booster::ptime::millisleep(100);
		io::stream_socket s;
		s.open(io::pf_inet);
		s.connect(io::endpoint("127.0.0.1",8080));
		char c;
		s.read(io::buffer(&c,1));
		TEST(c=='x');
		s.close();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

void async_socket_writer(sys::error_code const &e)
{
	TEST(!e);
	accepted_socket->write(io::buffer("x",1));
	the_service->stop();
	called=true;
}

void async_pid_writer(sys::error_code const &e)
{
	TEST(!e);
	int pid=getpid();
	accepted_socket->write(io::buffer(&pid,sizeof(pid)));
	booster::ptime::millisleep(100);
	accepted_socket->close();
	the_service->stop();
}

void child()
{
	io::stream_socket s(*the_service);
	accepted_socket=&s;
	the_socket->async_accept(s,async_pid_writer);
	the_service->run();
}

int main()
{
	int pid1 = 0;
	int pid2 = 0;
	try {
		std::cout << "Cancel before" << std::endl;
		io::io_service srv;
		the_service = &srv;
		io::acceptor ac(srv);
		the_socket = &ac;
		ac.open(io::pf_inet);
		ac.set_option(io::basic_socket::reuse_address,true);
		io::endpoint ep("127.0.0.1",8080);
		ac.bind(ep);
		ac.listen(5);
		io::stream_socket s1(srv);
		ac.async_accept(s1,socket_accept_failer);
		ac.cancel();
		srv.run();
		TEST(called); called=false;
		srv.reset();
		std::cout << "Cancel by timer" << std::endl;
		
		io::deadline_timer dt(srv);
		dt.expires_from_now(booster::ptime::milliseconds(100));
		dt.async_wait(socket_canceler);
		ac.async_accept(s1,socket_accept_failer);
		srv.run();
		TEST(called); called=false;
		srv.reset();

		std::cout << "Cancel by thread" << std::endl;
		booster::thread t1(thread_socket_canceler);
		ac.async_accept(s1,socket_accept_failer);
		srv.run();
		TEST(called); called=false;
		t1.join();
		srv.reset();
		
		std::cout << "Accept by thread" << std::endl;
		booster::thread t2(thread_socket_connector);
		ac.async_accept(s1,async_socket_writer);
		accepted_socket=&s1;
		srv.run();
		TEST(called); called=false;
		t2.join();
		srv.reset();
		s1.close();
		std::cout << "Acceptor is shared" << std::endl;
		
		pid1=fork();
		if(pid1==0) {
			child();
			return 0;
		}
		pid2=fork();
		if(pid2==0) {
			child();
			return 0;
		}

		booster::ptime::millisleep(100);
		s1.open(io::pf_inet);
		s1.connect(ep);
		int in_pid1=0;
		s1.read(io::buffer(&in_pid1,sizeof(int)));
		s1.close();
		s1.open(io::pf_inet);
		s1.connect(ep);
		int in_pid2=0;
		s1.read(io::buffer(&in_pid2,sizeof(int)));
		s1.close();

		TEST((in_pid1==pid1 && in_pid2==pid2) || (in_pid1==pid2 && in_pid2==pid1));
	
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		if(pid1!=0) {
			kill(pid1,SIGKILL);
			int r;
			wait(&r);
		}
		if(pid2!=0) {
			kill(pid2,SIGKILL);
			int r;
			wait(&r);
		}
		return 1;
	}
	if(pid1!=0 && pid2!=0) {
		int r1=0,r2=0;
		::wait(&r1);
		::wait(&r2);
		if(	WIFEXITED(r1) && WEXITSTATUS(r1)==0 
			&& WIFEXITED(r2) && WEXITSTATUS(r2)==0) 
		{
			std::cerr << "Ok" <<std::endl;
			return 0;
		}
		return 1;
	}
	return 0;
}
