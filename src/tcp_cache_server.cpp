///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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
#include <cppcms/session_storage.h>
#include <cppcms_boost/bind.hpp>
#include <booster/shared_ptr.h>
#include <booster/enable_shared_from_this.h>
#include <booster/thread.h>
#include <booster/aio/socket.h>
#include <booster/aio/io_service.h>
#include <booster/aio/endpoint.h>
#include <booster/aio/buffer.h>
#include <booster/aio/deadline_timer.h>
#include <booster/log.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "tcp_cache_server.h"

namespace boost = cppcms_boost;


namespace cppcms {
namespace impl {

namespace io = booster::aio;

class tcp_cache_service::session : public booster::enable_shared_from_this<tcp_cache_service::session> {
	std::vector<char> data_in_;
	std::string data_out_;
	cppcms::impl::tcp_operation_header hout_;
	cppcms::impl::tcp_operation_header hin_;

public:

	io::stream_socket socket_;
	booster::intrusive_ptr<cppcms::impl::base_cache> cache_;
	booster::shared_ptr<cppcms::sessions::session_storage> sessions_;


	session(io::io_service &srv,
		booster::intrusive_ptr<cppcms::impl::base_cache> c,
		booster::shared_ptr<cppcms::sessions::session_storage_factory> f) :
		socket_(srv),
		cache_(c)
	{
		if(f) {
			sessions_ = f->get();
		}
	}
	void run()
	{
		socket_.async_read(io::buffer(&hin_,sizeof(hin_)),
				boost::bind(&session::on_header_in,shared_from_this(),
						_1));
	}
	void on_header_in(booster::system::error_code const &e)
	{
		if(e) { handle_error(e); return; }
		data_in_.clear();
		data_in_.resize(hin_.size);
		if(hin_.size > 0) {
			socket_.async_read(io::buffer(data_in_),
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
		key.assign(data_in_.begin(),data_in_.end());
		if(hin_.operations.fetch.transfer_triggers)
			ptags=&tags;
		uint64_t generation;
		time_t timeout;
		if(!cache_->fetch(key,&a,ptags,&timeout,&generation)) {
			hout_.opcode=opcodes::no_data;
			return;
		}
		if(hin_.operations.fetch.transfer_if_not_uptodate 
			&& generation==hin_.operations.fetch.current_gen)
		{
			hout_.opcode=opcodes::uptodate;
			return;
		}
		hout_.opcode=opcodes::data;
		data_out_.swap(a);
		hout_.operations.data.data_len=data_out_.size();
		if(ptags) {
			for(std::set<std::string>::iterator p=tags.begin(),e=tags.end();p!=e;++p) {
				data_out_.append(p->c_str(),p->size()+1);
			}
		}
		hout_.operations.data.triggers_len=data_out_.size()-hout_.operations.data.data_len;
		hout_.size=data_out_.size();
		
		hout_.operations.data.generation=generation;
		hout_.operations.data.timeout = timeout;
	}

	void rise()
	{
		std::string key;
		key.assign(data_in_.begin(),data_in_.end());
		cache_->rise(key);
		hout_.opcode=opcodes::done;
	}
	void clear()
	{
		cache_->clear();
		hout_.opcode=opcodes::done;
	}
	void stats()
	{
		unsigned k,t;
		cache_->stats(k,t);
		hout_.opcode=opcodes::out_stats;
		hout_.operations.out_stats.keys=k;
		hout_.operations.out_stats.triggers=t;
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
		if(	hin_.operations.store.key_len
			+hin_.operations.store.data_len
			+hin_.operations.store.triggers_len != hin_.size
			|| hin_.operations.store.key_len == 0)
		{
			hout_.opcode=opcodes::error;
			return;
		}
		std::string ts;
		std::vector<char>::iterator p=data_in_.begin()
			+hin_.operations.store.key_len
			+hin_.operations.store.data_len;
		ts.assign(p,p + hin_.operations.store.triggers_len);
		if(!load_triggers(triggers,ts.c_str(),
					hin_.operations.store.triggers_len))
		{
			hout_.opcode=opcodes::error;
			return;
		}
		time_t timeout=to_time_t(hin_.operations.store.timeout);
		std::string key;
		key.assign(data_in_.begin(),data_in_.begin()+hin_.operations.store.key_len);
		std::string data;
		data.assign(data_in_.begin()+hin_.operations.store.key_len,
				data_in_.begin() + hin_.operations.store.key_len + hin_.operations.store.data_len);
		cache_->store(key,data,triggers,timeout);
		hout_.opcode=opcodes::done;
	}
	void save()
	{
		if(hin_.size < 32)
		{
			hout_.opcode=opcodes::error;
			return;
		}
		time_t timeout=to_time_t(hin_.operations.session_save.timeout);
		std::string sid(data_in_.begin(),data_in_.begin()+32);
		std::string value(data_in_.begin()+32,data_in_.end());
		sessions_->save(sid,timeout,value);
		hout_.opcode=opcodes::done;
	}
	void load()
	{
		if(hin_.size!=32) {
			hout_.opcode=opcodes::error;
			return;
		}
		time_t timeout;
		int toffset;
		std::string sid(data_in_.begin(),data_in_.end());
		if(!sessions_->load(sid,timeout,data_out_) || (toffset=(timeout)) < 0) {
			hout_.opcode=opcodes::no_data;
			return;
		}
		hout_.opcode=opcodes::session_load_data;
		hout_.size=data_out_.size();
		hout_.operations.session_data.timeout=toffset;
	}
	void remove()
	{
		if(hin_.size!=32) {
			hout_.opcode=opcodes::error;
			return;
		}
		std::string sid(data_in_.begin(),data_in_.end());
		sessions_->remove(sid);
	}
	void handle_error(booster::system::error_code const &e)
	{
		if(e.category() == booster::aio::aio_error_cat && e.value() == booster::aio::aio_error::eof) {
			BOOSTER_DEBUG("cppcms_scale") << "Client disconnected, fd=" << socket_.native() 
				<<"; " << e.message();
			return;
		}
		BOOSTER_WARNING("cppcms_scale") << "Error on connection, fd=" << socket_.native() 
				<<"; " << e.message();
	}
	void on_data_in(booster::system::error_code const &e)
	{
		if(e) {
			handle_error(e);
			return;
		}
		memset(&hout_,0,sizeof(hout_));
		BOOSTER_DEBUG("cppcms_scale") << "Received command " << hin_.opcode << "(" 
			<< opcodes::to_name(hin_.opcode) <<"); fd="<< socket_.native();
		switch(hin_.opcode) {
		case opcodes::fetch:
		case opcodes::rise:
		case opcodes::clear:
		case opcodes::store:
		case opcodes::stats:
			if(!cache_)
				hout_.opcode=opcodes::error;
			break;
		case opcodes::session_save:
		case opcodes::session_load:
		case opcodes::session_remove:
			if(!sessions_)
				hout_.opcode=opcodes::error;
			break;
		default:
			hout_.opcode=opcodes::error;
		}
		if(hout_.opcode!=opcodes::error) {
			switch(hin_.opcode){
			case opcodes::fetch:		fetch(); break;
			case opcodes::rise:		rise(); break;
			case opcodes::clear:		clear(); break;
			case opcodes::store:		store(); break;
			case opcodes::stats:		stats(); break;
			case opcodes::session_save:	save(); break;
			case opcodes::session_load:	load(); break;
			case opcodes::session_remove:	remove(); break;
			default:
				hout_.opcode=opcodes::error;
			}
		}
		BOOSTER_DEBUG("cppcms_scale") << "Returning answer " << hout_.opcode << "(" 
			<< opcodes::to_name(hout_.opcode) <<"); fd="<< socket_.native();
		io::const_buffer packet = io::buffer(&hout_,sizeof(hout_));
		if(hout_.size > 0) {
			packet += io::buffer(data_out_.c_str(),hout_.size);
		}
		socket_.async_write(packet,
			boost::bind(&session::on_data_out,shared_from_this(),
				_1));
	}
	void on_data_out(booster::system::error_code const &e)
	{
		if(e) { handle_error(e); return; }
		run();
	}

};

class tcp_cache_service::server  {
	io::acceptor acceptor_;
	size_t counter;
	booster::intrusive_ptr<cppcms::impl::base_cache> cache_;
	std::vector<io::io_service *> services_;
	booster::shared_ptr<cppcms::sessions::session_storage_factory> sessions_;
	void on_accept(booster::system::error_code const &e,booster::shared_ptr<tcp_cache_service::session> s)
	{
		if(!e) {
			BOOSTER_DEBUG("cppcms_scale") << "Accepted connection, fd=" << s->socket_.native();
			s->socket_.set_option(io::stream_socket::tcp_no_delay,true);
			if(&acceptor_.get_io_service()  == &s->socket_.get_io_service()) {
				s->run();
			}
			else {
				s->socket_.get_io_service().post(boost::bind(&session::run,s));
			}
			start_accept();
		}
		else {
			BOOSTER_ERROR("cppcms_scale") << "Failed to accept connection:" << e.message();
		}
	}
	io::io_service &get_next_io_service()
	{
		int id = counter++;
		if(counter >= services_.size())
			counter = 0;
		return *services_[id];
	}
	void start_accept()
	{
		booster::shared_ptr<session> s(new session(get_next_io_service(),cache_,sessions_));
		acceptor_.async_accept(s->socket_,boost::bind(&server::on_accept,this,_1,s));
	}
public:
	server(		std::vector<booster::shared_ptr<io::io_service> > &io,
			std::string ip,
			int port,
			booster::intrusive_ptr<cppcms::impl::base_cache> c,
			booster::shared_ptr<cppcms::sessions::session_storage_factory> f
		):
		acceptor_(*io[0]),
		counter(0),
		cache_(c),
		sessions_(f)
	{
		services_.resize(io.size());
		for(size_t i=0;i<io.size();i++)
			services_[i] = io[i].get();
		io::endpoint ep(ip,port);
		acceptor_.open(ep.family());
		acceptor_.set_option(io::basic_socket::reuse_address,true);
		acceptor_.bind(ep);
		acceptor_.listen(10);
		start_accept();
	}
};


class garbage_collector
{
public:
	garbage_collector(	booster::shared_ptr<cppcms::sessions::session_storage_factory> f,
				int seconds)
		:	timer_(srv_),
			io_(f),
			seconds_(seconds)
	{
	}
	void async_run(booster::system::error_code const &e)
	{
		if(e) return;
		
		timer_.expires_from_now(booster::ptime::seconds(seconds_));
		timer_.async_wait(boost::bind(&garbage_collector::async_run,this,_1));
		
		io_->gc_job();
	}
	void stop()
	{
		srv_.stop();
	}
	void run()
	{
		try {
			async_run(booster::system::error_code());
			srv_.run();
		}
		catch(std::exception const &e) {
			BOOSTER_ERROR("cppcms_scale") << "garbage_collector::run: " << 
				e.what() << booster::trace(e);
		}
	}
private:

	booster::aio::io_service srv_;
	booster::aio::deadline_timer timer_;
	booster::shared_ptr<cppcms::sessions::session_storage_factory> io_;
	int seconds_;
};


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
				BOOSTER_ERROR("cppcms_scale") << "Error:" << e.what() << booster::trace(e);
			}
		}
	}
	catch(std::exception const &e)
	{
		BOOSTER_ERROR("cppcms_scale") << "Fatal:" << e.what() << booster::trace(e);
	}
	catch(...){
		BOOSTER_ERROR("cppcms_scale") << "Unknown exception" << std::endl;
	}
}

struct tcp_cache_service::_data {
	std::vector<booster::shared_ptr<io::io_service> > io;
	std::auto_ptr<server> srv_cache;
	booster::intrusive_ptr<base_cache> cache;
	std::vector<booster::shared_ptr<booster::thread> > threads;
	booster::shared_ptr<booster::thread> gc_thread;
	booster::shared_ptr<garbage_collector> gc_runner;
};

tcp_cache_service::tcp_cache_service(	booster::intrusive_ptr<base_cache> cache,
					booster::shared_ptr<cppcms::sessions::session_storage_factory> factory,
					int threads,
					std::string ip,
					int port,
					int gc_timeout) :
	d(new _data)
{
	d->io.resize(threads);
	for(int i=0;i<threads;i++) {
		d->io[i].reset(new io::io_service());
	}
	d->cache=cache;
	d->srv_cache.reset(new server(d->io,ip,port,cache,factory));
#ifndef CPPCMS_WIN32
	sigset_t new_mask;
	sigfillset(&new_mask);
	sigset_t old_mask;
	pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);
#endif
	if(factory && factory->requires_gc()) {
		d->gc_runner.reset(new garbage_collector(factory,gc_timeout));
		d->gc_thread.reset(new booster::thread(boost::bind(&garbage_collector::run,d->gc_runner)));
	}

	for(int i=0;i<threads;i++){
		booster::shared_ptr<booster::thread> thread;
		thread.reset(new booster::thread(boost::bind(thread_function,d->io[i].get())));
		d->threads.push_back(thread);
	}
#ifndef CPPCMS_WIN32
	// Restore previous mask
	pthread_sigmask(SIG_SETMASK,&old_mask,0);
#endif
}

void tcp_cache_service::stop()
{
	for(size_t i=0;i<d->io.size();i++)
		d->io[i]->stop();
	if(d->gc_runner) {
		d->gc_runner->stop();
	}
}

tcp_cache_service::~tcp_cache_service()
{
	try {
		stop();
		for(unsigned i=0;i<d->threads.size();i++)
			d->threads[i]->join();
		if(d->gc_thread)
			d->gc_thread->join();
		d->srv_cache.reset();
	}
	catch(...){}
}

} // impl
} // cppcms
