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
#define CPPCMS_SOURCE
// MUST BE FIRST TO COMPILE CORRECTLY UNDER CYGWIN
#include <cppcms/defs.h>
#ifndef CPPCMS_WIN32
#if defined(__sun)
#define _POSIX_PTHREAD_SEMANTICS
#endif
#include <signal.h>
#endif

#include "tcp_cache_protocol.h"
#include "cache_storage.h"
#include <cppcms/cppcms_error.h>
#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif
#include <booster/shared_ptr.h>
#include <booster/enable_shared_from_this.h>
#include <booster/thread.h>
#include <booster/aio/socket.h>
#include <booster/aio/io_service.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/buffer.h>
#include <time.h>
#include <stdlib.h>

#include <iostream>

#include "tcp_cache_server.h"

namespace cppcms {
namespace impl {

namespace io = booster::aio;

class tcp_cache_service::session : public booster::enable_shared_from_this<tcp_cache_service::session> {
	std::vector<char> data_in;
	std::string data_out;
	cppcms::impl::tcp_operation_header hout;
	cppcms::impl::tcp_operation_header hin;

public:
	io::socket socket_;
	cppcms::impl::base_cache &cache;
	//cppcms::session_storage &sessions;
	session(io::io_service &srv,cppcms::impl::base_cache &c): //,session_server_storage &s) : 
		socket_(srv), cache(c) //,sessions(s)
	{
	}
	void run()
	{
		socket_.async_read(io::buffer(&hin,sizeof(hin)),
				boost::bind(&session::on_header_in,shared_from_this(),
						_1));
	}
	void on_header_in(booster::system::error_code const &e)
	{
		if(e) return;
		data_in.clear();
		data_in.resize(hin.size);
		if(hin.size > 0) {
			socket_.async_read(io::buffer(data_in),
				boost::bind(&session::on_data_in,shared_from_this(),
						_1));
		}
		else {
			on_data_in(e);
		}
	}
	
	void fetch()
	{
		std::string a;
		std::set<std::string> tags,*ptags=0;
		std::string key;
		key.assign(data_in.begin(),data_in.end());
		if(hin.operations.fetch.transfer_triggers)
			ptags=&tags;
		uint64_t generation;
		time_t timeout;
		if(!cache.fetch(key,&a,ptags,&timeout,&generation)) {
			hout.opcode=opcodes::no_data;
			return;
		}
		if(hin.operations.fetch.transfer_if_not_uptodate 
			&& generation==hin.operations.fetch.current_gen)
		{
			hout.opcode=opcodes::uptodate;
			return;
		}
		hout.opcode=opcodes::data;
		data_out.swap(a);
		hout.operations.data.data_len=data_out.size();
		if(ptags) {
			for(std::set<std::string>::iterator p=tags.begin(),e=tags.end();p!=e;++p) {
				data_out.append(p->c_str(),p->size()+1);
			}
		}
		hout.operations.data.triggers_len=data_out.size()-hout.operations.data.data_len;
		hout.size=data_out.size();
		
		hout.operations.data.generation=generation;
		time_t now=time(0);
		hout.operations.data.timeout = timeout > now ? timeout - now : 0;
	}

	void rise()
	{
		std::string key;
		key.assign(data_in.begin(),data_in.end());
		cache.rise(key);
		hout.opcode=opcodes::done;
	}
	void clear()
	{
		cache.clear();
		hout.opcode=opcodes::done;
	}
	void stats()
	{
		unsigned k,t;
		cache.stats(k,t);
		hout.opcode=opcodes::out_stats;
		hout.operations.out_stats.keys=k;
		hout.operations.out_stats.triggers=t;
	}
	bool load_triggers(std::set<std::string> &triggers,char const *start,unsigned len)
	{
		int slen=len;
		while(slen>0) {
			unsigned size=strlen(start);
			if(size==0) {
				return false;
			}
			std::string tmp;
			tmp.assign(start,size);
			slen-=size+1;
			start+=size+1;
			triggers.insert(tmp);
		}
		return true;
	}
	void store()
	{
		std::set<std::string> triggers;
		if(	hin.operations.store.key_len
			+hin.operations.store.data_len
			+hin.operations.store.triggers_len != hin.size
			|| hin.operations.store.key_len == 0)
		{
			hout.opcode=opcodes::error;
			return;
		}
		std::string ts;
		std::vector<char>::iterator p=data_in.begin()
			+hin.operations.store.key_len
			+hin.operations.store.data_len;
		ts.assign(p,p + hin.operations.store.triggers_len);
		if(!load_triggers(triggers,ts.c_str(),
					hin.operations.store.triggers_len))
		{
			hout.opcode=opcodes::error;
			return;
		}
		time_t timeout=time(0)+(time_t)hin.operations.store.timeout;
		std::string key;
		key.assign(data_in.begin(),data_in.begin()+hin.operations.store.key_len);
		std::string data;
		data.assign(data_in.begin()+hin.operations.store.key_len,
				data_in.begin() + hin.operations.store.key_len + hin.operations.store.data_len);
		cache.store(key,data,triggers,timeout);
		hout.opcode=opcodes::done;
	}
/*	
	void save()
	{
		if(hin.size <= 32)
		{
			hout.opcode=opcodes::error;
			return;
		}
		time_t timeout=hin.operations.session_save.timeout + time(NULL);
		string sid(data_in.begin(),data_in.begin()+32);
		string value(data_in.begin()+32,data_in.end());
		sessions.save(sid,timeout,value);
		hout.opcode=opcodes::done;
	}
	void load()
	{
		if(hin.size!=32) {
			hout.opcode=opcodes::error;
			return;
		}
		time_t timeout;
		int toffset;
		string sid(data_in.begin(),data_in.end());
		if(!sessions.load(sid,&timeout,data_out) && (toffset=(timeout-time(NULL))) < 0) {
			hout.opcode=opcodes::no_data;
			return;
		}
		hout.opcode=opcodes::session_load_data;
		hout.size=data_out.size();
		hout.operations.session_data.timeout=toffset;
	}
	void remove()
	{
		if(hin.size!=32) {
			hout.opcode=opcodes::error;
			return;
		}
		string sid(data_in.begin(),data_in.end());
		sessions.remove(sid);
	}
	*/
	void on_data_in(booster::system::error_code const &e)
	{
		if(e) return;
		memset(&hout,0,sizeof(hout));
		switch(hin.opcode){
		case opcodes::fetch:		fetch(); break;
		case opcodes::rise:		rise(); break;
		case opcodes::clear:		clear(); break;
		case opcodes::store:		store(); break;
		case opcodes::stats:		stats(); break;
		/*case opcodes::session_save:	save(); break;
		case opcodes::session_load:	load(); break;
		case opcodes::session_remove:	remove(); break;*/
		default:
			hout.opcode=opcodes::error;
		}
		socket_.async_write(io::buffer(&hout,sizeof(hout)),
			boost::bind(&session::on_header_out,shared_from_this(),
				_1));
	}
	void on_header_out(booster::system::error_code const &e)
	{
		if(e) return;
		if(hout.size==0) {
			run();
			return ;
		}
		socket_.async_write(io::buffer(data_out.c_str(),hout.size),
			boost::bind(&session::on_data_out,shared_from_this(),
				_1));
	}
	void on_data_out(booster::system::error_code const &e)
	{
		if(e) return;
		run();
	}

};

class tcp_cache_service::server  {
	io::socket acceptor_;
	cppcms::impl::base_cache &cache;
	//session_server_storage &sessions;
	void on_accept(booster::system::error_code const &e,booster::shared_ptr<tcp_cache_service::session> s)
	{
		if(!e) {
			s->socket_.set_option(io::socket::tcp_no_delay,true);
			s->run();
			start_accept();
		}
	}
	void start_accept()
	{
		//booster::shared_ptr<session> s(new session(acceptor_.io_service(),cache,sessions));
		booster::shared_ptr<session> s(new session(acceptor_.get_io_service(),cache));
		acceptor_.async_accept(s->socket_,boost::bind(&server::on_accept,this,_1,s));
	}
public:
	server(	io::io_service &io,
		std::string ip,
		int port,
		cppcms::impl::base_cache &c
		//,session_server_storage &s
		) : 
		acceptor_(io),
		cache(c)
	//	,sessions(s)
	{
		io::endpoint ep(ip,port);
		acceptor_.open(ep.family(),io::sock_stream);
		acceptor_.set_option(io::socket::reuse_address,true);
		acceptor_.bind(ep);
		acceptor_.listen(10);
		start_accept();
	}
};

/*
class garbage_collector
{
	aio::deadline_timer timer;
	booster::shared_ptr<storage::io> io;
	int seconds;
	void submit()
	{
		timer.expires_from_now(boost::posix_time::seconds(seconds));
		timer.async_wait(boost::bind(&garbage_collector::gc,this,_1));
	}
	void gc(error_code const &e)
	{
		session_file_storage::gc(io);
		submit();
	}
public:
	garbage_collector(io::io_service &srv,int sec,booster::shared_ptr<storage::io> io_) :
		timer(srv),
		seconds(sec),
		io(io_)
	{
		submit();	
	}
};
*/

static void thread_function(io::io_service *io)
{
	bool stop=false;
	try{
		while(!stop) {
			try {
				io->run();
				stop=true;
			}
			catch(cppcms::cppcms_error const &e) {
				// Not much to do...
				// Object will be destroyed automatically 
				// Because it does not resubmit itself
				std::cerr <<"CppCMS Error "<<e.what()<<std::endl;
			}
		}
	}
	catch(std::exception const &e)
	{
		std::cerr << "Fatal" << e.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown exception" << std::endl;
	}
	io->stop();
}

struct tcp_cache_service::data {
	io::io_service io;
	std::auto_ptr<server> srv_cache;
	booster::intrusive_ptr<base_cache> cache;
	std::vector<booster::shared_ptr<booster::thread> > threads;
};

tcp_cache_service::tcp_cache_service(booster::intrusive_ptr<base_cache> cache,int threads,std::string ip,int port) :
	d(new data)
{
	d->cache=cache;
	d->srv_cache.reset(new server(d->io,ip,port,*cache));
#ifndef CPPCMS_WIN32
	sigset_t new_mask;
	sigfillset(&new_mask);
	sigset_t old_mask;
	pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif

	int i;
	for(i=0;i<threads;i++){
		booster::shared_ptr<booster::thread> thread;
		thread.reset(new booster::thread(boost::bind(thread_function,&d->io)));
		d->threads.push_back(thread);
	}
#ifndef CPPCMS_WIN32
	// Restore previous mask
	pthread_sigmask(SIG_SETMASK,&old_mask,0);
#endif
}

void tcp_cache_service::stop()
{
	d->io.stop();
}

tcp_cache_service::~tcp_cache_service()
{
	d->srv_cache.reset();
}

} // impl
} // cppcms
