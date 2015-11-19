///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_DUMMY_API_H
#define CPPCMS_IMPL_DUMMY_API_H
#include "cgi_api.h"

using cppcms::impl::cgi::io_handler;
using cppcms::impl::cgi::handler;
using cppcms::impl::cgi::callback;


class dummy_api  : public cppcms::impl::cgi::connection {
public:
	dummy_api(cppcms::service &srv,std::map<std::string,std::string> env,std::string &output,bool mark_chunks=false,bool write_eof=false) :
		cppcms::impl::cgi::connection(srv),
		output_(&output),
		write_eof_(write_eof),
		mark_chunks_(mark_chunks)
	{
		for(std::map<std::string,std::string>::iterator p=env.begin();p!=env.end();++p)
			env_.add(pool_.add(p->first),pool_.add(p->second));
	}

	booster::aio::const_buffer format_output(booster::aio::const_buffer const &in,bool,booster::system::error_code &)
	{
		return in;
	}
	void async_read_headers(handler const &) 
	{
		throw std::runtime_error("dummy_api: unsupported");
	}

	void async_read_eof(callback const &) 
	{
		throw std::runtime_error("dummy_api: unsupported");
	}

	virtual void do_eof(){}
	virtual void on_some_output_written() {}
	virtual bool write(booster::aio::const_buffer const &in,bool eof,booster::system::error_code &) 
	{
		std::pair<booster::aio::const_buffer::entry const *,size_t> all=in.get();
		for(size_t i=0;i<all.second;i++)  {
			if(mark_chunks_)
				output_->append("[");
			output_->append(reinterpret_cast<char const *>(all.first[i].ptr),all.first[i].size);
			if(mark_chunks_)
				output_->append("]");
		}
		if(eof && write_eof_)
			output_->append("[EOF]");
		return true;
	}
	virtual bool nonblocking_write(booster::aio::const_buffer const &in,bool eof,booster::system::error_code &e)
	{
		return write(in,eof,e);
	}
	virtual booster::aio::stream_socket &socket()
	{
		throw std::runtime_error("dummy_api: unsupported");
	}
	virtual booster::aio::io_service &get_io_service() 
	{
		throw std::runtime_error("dummy_api: unsupported");
	}
	bool keep_alive()
	{
		return false;
	}
	void close(){}
	virtual void async_read_some(void *,size_t,io_handler const &) 
	{
		throw std::runtime_error("dummy_api: unsupported");
	}
private:
	std::string *output_;
	bool write_eof_;
	bool mark_chunks_;

};


#endif
