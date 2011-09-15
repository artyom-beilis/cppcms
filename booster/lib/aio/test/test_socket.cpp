//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/aio/socket.h>
#include <booster/aio/buffer.h>
#include <booster/aio/io_service.h>
#include <booster/aio/deadline_timer.h>
#include <booster/system_error.h>
#include <booster/aio/reactor.h>
#include <booster/thread.h>
#include <booster/posix_time.h>
#include <iostream>
#include <set>
#include "test.h"

#ifndef BOOSTER_WIN32
#include <unistd.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace io=booster::aio;
namespace sys=booster::system;

using booster::ptime;

void make_pair(io::stream_socket &s1,io::stream_socket &s2)
{
	io::acceptor a;
	a.open(io::pf_inet);
	a.set_option(io::stream_socket::reuse_address,true);
	a.bind(io::endpoint("127.0.0.1",0));
	a.listen(1);
	s1.open(io::pf_inet);
	s1.connect(a.local_endpoint());
	a.accept(s2);
	a.close();
}

void test_connected(io::stream_socket &s1,io::stream_socket &s2)
{
	TEST(s1.write_some(io::buffer("x",1))==1);
	char c;
	TEST(s2.read_some(io::buffer(&c,1))==1 && c=='x');	
}

void test_buffer()
{
	std::cout << "Testing buffers" << std::endl;
	io::const_buffer b;
	char const *s1="test";
	char const *s2="";
	char const *s3="foobar";
	char const *s4="bee";
	TEST(b.empty());
	b.add(s2,0);
	TEST(b.empty());
	b.add(s1,4);
	TEST(!b.empty());
	TEST(b.get().second==1);
	TEST(b.get().first[0].ptr==s1 && b.get().first[0].size==4);
	b.add(s3,6);
	TEST(b.get().second==2);
	TEST(b.get().first[0].ptr==s1 && b.get().first[0].size==4);
	TEST(b.get().first[1].ptr==s3 && b.get().first[1].size==6);
	b.add(s4,3);
	TEST(b.get().second==3);
	TEST(b.get().first[0].ptr==s1 && b.get().first[0].size==4);
	TEST(b.get().first[1].ptr==s3 && b.get().first[1].size==6);
	TEST(b.get().first[2].ptr==s4 && b.get().first[2].size==3);
	{
		io::const_buffer b2=b;
		TEST(b2.get().second==3);
		TEST(b2.get().first[0].ptr==s1 && b2.get().first[0].size==4);
		TEST(b2.get().first[1].ptr==s3 && b2.get().first[1].size==6);
		TEST(b2.get().first[2].ptr==s4 && b2.get().first[2].size==3);
	}
	{
		io::const_buffer b2=io::buffer(s1,4)+io::buffer(s2,0)+io::buffer(s3,6) + io::buffer(s4,3);
		TEST(b2.get().second==3);
		TEST(b2.get().first[0].ptr==s1 && b2.get().first[0].size==4);
		TEST(b2.get().first[1].ptr==s3 && b2.get().first[1].size==6);
		TEST(b2.get().first[2].ptr==s4 && b2.get().first[2].size==3);
	}
	{
		io::const_buffer b2;
		b2+=io::buffer(s1,4);
		b2+=io::buffer(s2,0);
		b2+=io::buffer(s3,6);
		b2+=io::buffer(s4,3);
		TEST(b2.get().second==3);
		TEST(b2.get().first[0].ptr==s1 && b2.get().first[0].size==4);
		TEST(b2.get().first[1].ptr==s3 && b2.get().first[1].size==6);
		TEST(b2.get().first[2].ptr==s4 && b2.get().first[2].size==3);
	}
	{
		io::const_buffer b2=b+3;
		TEST(b2.get().second==3);
		TEST(b2.get().first[0].ptr==s1+3 && b2.get().first[0].size==1);
		TEST(b2.get().first[1].ptr==s3 && b2.get().first[1].size==6);
		TEST(b2.get().first[2].ptr==s4 && b2.get().first[2].size==3);
	}
	{
		io::const_buffer b2=b+4;
		TEST(b2.get().second==2);
		TEST(b2.get().first[0].ptr==s3 && b2.get().first[0].size==6);
		TEST(b2.get().first[1].ptr==s4 && b2.get().first[1].size==3);
	}
	{
		io::const_buffer b2=b+5;
		TEST(b2.get().second==2);
		TEST(b2.get().first[0].ptr==s3+1 && b2.get().first[0].size==5);
		TEST(b2.get().first[1].ptr==s4 && b2.get().first[1].size==3);
	}
	{
		io::const_buffer b2=b+9;
		TEST(b2.get().second==2);
		TEST(b2.get().first[0].ptr==s3+5 && b2.get().first[0].size==1);
		TEST(b2.get().first[1].ptr==s4 && b2.get().first[1].size==3);
	}
	{
		io::const_buffer b2=b+12;
		TEST(b2.get().second==1);
		TEST(b2.get().first[0].ptr==s4+2 && b2.get().first[0].size==1);
	}
	{
		io::const_buffer b2=b+13;
		TEST(b2.empty());
		TEST(b2.get().second==0);
	}
	{
		io::const_buffer b2=b+14;
		TEST(b2.empty());
		TEST(b2.get().second==0);
	}
	char buf[3]={ 'x', 'y' ,'z' };
	io::mutable_buffer mb=io::buffer(buf,3);
	io::const_buffer cb=mb;
	TEST(cb.get().second==1);
	TEST(cb.get().first[0].ptr==buf && cb.get().first[0].size==3);
}

void basic_io()
{
	std::cout << "Test basic io" << std::endl;
	std::string str1="hello";
	io::stream_socket s1,s2;
	make_pair(s1,s2);
	TEST(s1.write_some(booster::aio::buffer(str1))==5);
	char buf[16] = {0};
	TEST(s2.read_some(booster::aio::buffer(buf,sizeof(buf)))==5);
	TEST(str1==buf);
	str1="x";
	TEST(s2.write_some(io::buffer(str1))==1);
	TEST(s1.read_some(io::buffer(buf,sizeof(buf)))==1 && buf[0]=='x');
	s1.shutdown(io::stream_socket::shut_wr);
	sys::error_code e;
	TEST(s2.read_some(io::buffer(buf,1),e)==0 && e.value()==io::aio_error::eof);
	char c='y';
	TEST(s2.write_some(io::buffer(&c,1))==1);
	s2.shutdown(io::stream_socket::shut_wr);
	c=0;
	TEST(s1.read_some(io::buffer(&c,1))==1 && c=='y');
	e=sys::error_code();
	TEST(s1.read_some(io::buffer(&c,1),e)==0 && e.value()==io::aio_error::eof);
}

void test_unix()
{
#if !defined(BOOSTER_AIO_NO_PF_UNIX)
	std::cout << "Testing Unix Sockets" << std::endl;
	io::acceptor acc;
	acc.open(io::pf_unix);
	io::endpoint ep("booster_unix_test_socket");
	::unlink(ep.path().c_str());
	acc.bind(ep);
	acc.listen(1);
	io::stream_socket s1;
	io::stream_socket s2;
	s1.open(io::pf_unix);
	s1.connect(ep);
	acc.accept(s2);
	test_connected(s1,s2);
	s1.close();
	s2.close();
	acc.close();
	::unlink(ep.path().c_str());
#endif
}

void test_ipv6()
{
#if !defined(BOOSTER_AIO_NO_PF_INET6)
	std::cout << "Testing IPv6" << std::endl;
	io::acceptor acc;
	sys::error_code err;
	acc.open(io::pf_inet6,err);
	if(err) {
		std::cerr << "-- Looks like IPv6 is not supported on this host" << std::endl;
		return;
	}
	io::endpoint ep("::1",8080);
	acc.bind(ep);
	acc.listen(1);
	io::stream_socket s1;
	io::stream_socket s2;
	s1.open(io::pf_inet6);
	s1.connect(ep);
	acc.accept(s2);
	test_connected(s1,s2);
	s1.close();
	s2.close();
	acc.close();
#endif
}



void readv_writev()
{
	return;
	std::cout << "Test writev/readv " << std::endl;
	io::stream_socket s1;
	io::stream_socket s2;
	std::string str1="hello ";
	std::string str2="world";
	std::string str=str1+str2;
	io::const_buffer b;
	b.add(str1.c_str(),str1.size());
	b.add(str2.c_str(),str2.size());

	make_pair(s1,s2);
	TEST(s1.write_some(b)==str.size());
	std::vector<char> tmp(16,0);
	TEST(s2.read_some(io::buffer(tmp))==str.size());
	TEST(str==&tmp.front());
}

void get_set_srv()
{
	std::cout << "Test get/set io_service" << std::endl;
	io::io_service srv;
	io::stream_socket s1(srv);
	TEST(s1.has_io_service());
	TEST(&s1.get_io_service() == &srv);
	s1.reset_io_service();
	TEST(!s1.has_io_service());
	try {s1.get_io_service(); TEST(!"Shoud throw"); }
	catch(sys::system_error const &e) {}
	io::stream_socket s2;
	TEST(!s2.has_io_service());
	try {s2.get_io_service(); TEST(!"Shoud throw"); }
	catch(sys::system_error const &e) {}
	s1.set_io_service(srv);
	s2.set_io_service(srv);
	TEST(&s1.get_io_service() == &srv);
	TEST(&s2.get_io_service() == &srv);
}

static io::io_service *the_service = 0;
static int calls_no = 0;
void reset_glb(io::io_service &srv) 
{
	the_service = &srv;
	calls_no = 1;
}

void async_connect_handler(sys::error_code const &e)
{
	TEST(!e);
	calls_no *=2;
	if(calls_no == 6)
		the_service->stop();
}

void async_accept_handler(sys::error_code const &e)
{
	TEST(!e);
	calls_no *=3;
	if(calls_no == 6)
		the_service->stop();
}

void test_async_connect()
{
	std::cout << "Test async connect" << std::endl;
	io::io_service srv;
	reset_glb(srv);
	io::acceptor acc(srv);
	io::stream_socket s1(srv);
	acc.open(io::pf_inet);
	acc.set_option(io::stream_socket::reuse_address,true);
	acc.bind(io::endpoint("127.0.0.1",8080));
	acc.listen(1);
	acc.async_accept(s1,async_accept_handler);
	io::stream_socket s2(srv);
	s2.open(io::pf_inet);
	s2.async_connect(io::endpoint("127.0.0.1",8080),async_connect_handler);
	srv.run();
	TEST(calls_no == 6);
	s1.set_non_blocking(false);
	s2.set_non_blocking(false);
	test_connected(s1,s2);
}

void test_pair()
{
	std::cout << "Testing socket_pair" << std::endl;
	io::stream_socket s1,s2;
	io::socket_pair(s1,s2);
	test_connected(s1,s2);
}


struct async_reader {
	int counter;
	io::stream_socket *sock;
	std::vector<char> *buf;
	void run()
	{
		sock->async_read_some(io::buffer(*buf),*this);
	}
	void operator()(sys::error_code const &e,size_t n)
	{
		if(e) {
			TEST(n==0);
			TEST(counter == 100000);
			TEST(e.value() == io::aio_error::eof);
			calls_no *= 2;
			if(calls_no == 6)
				the_service->stop();
		}
		else {
			TEST(counter < 100000);
			TEST(n!=0);
			for(unsigned i=0;i<n;i++)
				TEST(buf->at(i)==char((counter + i) % 100));
			counter+=n;
			run();
		}
	}
};

struct async_writer {
	int counter;
	io::stream_socket *sock;
	std::vector<char> *buf;
	void run()
	{
		unsigned i;
		for(i=0;i<buf->size() && counter+i < 100000;i++)
			buf->at(i)=char((counter + i) % 100);
		if(i < buf->size())
			buf->resize(i);
		sock->async_write_some(io::buffer(*buf),*this);
	}
	void operator()(sys::error_code const &e,size_t n)
	{
		TEST(!e);
		TEST(n>0);
		counter+=n;
		
		TEST(counter <= 100000);
		
		if(counter == 100000) {
			sock->shutdown(io::stream_socket::shut_rdwr);
			calls_no*=3;
			if(calls_no == 6)
				the_service->stop();
		}
		else {
			run();
		}
	}
};


void test_async_send_recv()
{
	std::cout << "Test async send/recv" << std::endl;
	io::io_service srv;
	io::stream_socket s1(srv),s2(srv);
	reset_glb(srv);
	make_pair(s1,s2);
	std::vector<char> in(151,0),out(190,0);
	async_reader rdr = { 0, &s1, &in };
	async_writer wrt = { 0, &s2, &out };
	rdr.run();
	wrt.run();
	srv.run();
	TEST(calls_no == 6);
}

bool cancel_op = false;

struct canceler {
	io::stream_socket *s;
	void operator()(sys::error_code const &e)
	{
		TEST(!e);
		s->cancel();
		cancel_op = true;
	}
};

struct cancel_once {
	cancel_once(io::stream_socket *s=0,bool b1=false,bool b2=false) : sock(s),write(b1),both(b2)
	{
	}
	
	io::stream_socket *sock;
	bool write;
	bool both;
	void operator()(sys::error_code const &e,size_t /*n*/)
	{
		static char buffer[1000];
		if(e || cancel_op) {
			TEST(cancel_op || e.value() == io::aio_error::canceled);
			if(write)
				calls_no*=2;
			else
				calls_no*=3;
			if(both) {
				if(calls_no % 2 == 0 && calls_no % 3 == 0)
					the_service->stop();
			}
			else {
				if(write) {
					if(calls_no % 2 == 0)
						the_service->stop();
				}
				else {
					if(calls_no % 3 == 0)
						the_service->stop();
				}
			}
		}
		else {
			if(write) {
				sock->async_write_some(io::buffer(buffer,sizeof(buffer)),*this);
			}
			else {
				sock->async_read_some(io::buffer(buffer,sizeof(buffer)),*this);
			}
		}
	}
};



void test_cancel_operations()
{
	{
		std::cout << "Test cancel write" << std::endl;
		io::io_service srv;
		reset_glb(srv);
		cancel_op = false;
		io::stream_socket s1(srv),s2(srv);
		make_pair(s1,s2);

		cancel_once co(&s1,true ,false);
		co(sys::error_code(),0);

		io::deadline_timer t(srv);
		t.expires_from_now(ptime::milliseconds(100));
		canceler c={&s1};
		t.async_wait(c);

		srv.run();
		TEST(calls_no==2);
	}
	{
		std::cout << "Test cancel read" << std::endl;
		io::io_service srv;
		reset_glb(srv);
		cancel_op = false;
		io::stream_socket s1(srv),s2(srv);
		make_pair(s1,s2);
		io::deadline_timer t(srv);
		t.expires_from_now(ptime::milliseconds(100));
		canceler c={&s1};
		t.async_wait(c);
		{
			cancel_once co(&s1,false,false);
			co(sys::error_code(),0);
		}
		srv.run();
		TEST(calls_no==3);
	}
	{
		std::cout << "Test cancel read and write" << std::endl;
		io::io_service srv;
		reset_glb(srv);
		cancel_op = false;
		io::stream_socket s1(srv),s2(srv);
		make_pair(s1,s2);
		s2.write_some(io::buffer(std::string("To be or not to be")));
		io::deadline_timer t(srv);
		t.expires_from_now(ptime::milliseconds(100));
		canceler c={&s1};
		t.async_wait(c);
		cancel_once co1(&s1,true,true);
		cancel_once co2(&s1,false,true);
		co1(sys::error_code(),0);
		co2(sys::error_code(),0);
		srv.run();
		TEST(calls_no==6);
	}
}


static char in[500000];
static char out[500000];

void prepare_in_out()
{
	::memset(out,0,sizeof(out));
	for(unsigned i=0;i<sizeof(in);i++) {
		in[i]=i*133+34;
	}
}

void test_in_out()
{
	TEST(memcmp(in,out,sizeof(in))==0);
}

bool thread_failed = false;

struct syn_ioer {
	io::stream_socket *self;
	char *ptr;
	size_t size;
	bool reading;
	void operator()()
	{
		try {
			size_t n=0;
			if(reading)
				n = self->read(io::buffer(ptr,size));
			else
				n = self->write(io::buffer(ptr,size));
			TEST(n==size);
		}
		catch(...) {
			thread_failed = true;
		}
	}
};

void after_async_read(sys::error_code const &e,size_t n)
{
	TEST(!e);
	TEST(n==sizeof(out));
	test_in_out();
	calls_no*=2;
	if(calls_no % 6 == 0)
		the_service->stop();
}

void after_async_write(sys::error_code const &e,size_t n)
{
	TEST(!e);
	TEST(n==sizeof(out));
	calls_no*=3;
	if(calls_no % 6 == 0)
		the_service->stop();
}

void test_full_read_write()
{
	std::cout << "Testing socket::read, socket::write" << std::endl;
	io::stream_socket s1,s2;
	make_pair(s1,s2);
	prepare_in_out();
	syn_ioer reader = {&s1,out,sizeof(out),true };
	syn_ioer writer = {&s2,in,sizeof(in),false };
	booster::thread th1(reader);
	booster::thread th2(writer);
	th1.join();
	th2.join();
	test_in_out();
	TEST(!thread_failed);

}

void test_full_async_read_write()
{
	std::cout << "Testing socket::async_read, socket::aync_write" << std::endl;
	io::io_service srv;
	io::stream_socket s1(srv),s2(srv);
	reset_glb(srv);
	make_pair(s1,s2);
	prepare_in_out();
	s1.async_write(io::buffer(in,sizeof(in)),after_async_write);
	s2.async_read(io::buffer(out,sizeof(out)),after_async_read);
	srv.run();
	TEST(calls_no == 6);
}

void got_erro_handler(sys::error_code const &e,size_t /*n*/)
{
	TEST(e);
	calls_no ++;
	the_service->stop();
}

struct socket_closer {
	io::stream_socket *s_;
	void operator()() const
	{
		sys::error_code e;
		booster::ptime::millisleep(200);
		s_->close(e);
		if(e) {
			std::cerr<<"Failed to close:"<<e.message()<<std::endl;
			exit(1);
		}
	}
};

void test_close()
{
	std::set<std::string> tested_;
	std::cout << "Testing close socket for reactors:" << std::endl;
	for(int type=io::reactor::use_default;type<=io::reactor::use_max;type++) {
		io::io_service srv(type);
		std::string name = srv.reactor_name();
		if(tested_.find(name)!=tested_.end())
			continue;
		tested_.insert(name);
		std::cout << "- " << name << " reactor"<<std::endl;
		
		{
			std::cout << "-- Cancel invalid/after close" << std::endl;
			reset_glb(srv);
			io::stream_socket s1(srv),s2(srv);
			make_pair(s1,s2);
			s1.close();
			char c;
			s1.async_read_some(io::buffer(&c,1),got_erro_handler);
			srv.run();
			TEST(calls_no == 2);
		}
		srv.reset();
		{
			std::cout << "-- Cancel close outside poll" << std::endl;
			reset_glb(srv);
			io::stream_socket s1(srv),s2(srv);
			make_pair(s1,s2);
			char c;
			s1.async_read_some(io::buffer(&c,1),got_erro_handler);
			s1.close();
			srv.run();
			TEST(calls_no == 2);
		}
		srv.reset();
		{
			std::cout << "-- Cancel close during poll" << std::endl;
			reset_glb(srv);
			io::stream_socket s1(srv),s2(srv);
			make_pair(s1,s2);
			char c;
			s1.async_read_some(io::buffer(&c,1),got_erro_handler);
			socket_closer closer = { &s1 };
			booster::thread t(closer);
			sys::error_code e;
			srv.run(e);
			t.join();
			TEST(!e);
			TEST(calls_no == 2);
		}
	}
	std::cout << std::endl;
}


int main()
{
	try {
		test_buffer();
		basic_io();
		readv_writev();
		get_set_srv();
		test_async_connect();
		test_unix();
		test_ipv6();
		test_pair();
		test_async_send_recv();
		test_cancel_operations();
		test_full_read_write();
		test_full_async_read_write();
		test_close();
	}

	catch(std::exception const &e)
	{
		std::cout<<"Failed:"<< e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
