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
	dummy_api(cppcms::service &srv,std::map<std::string,std::string> env,std::string &output) :
		cppcms::impl::cgi::connection(srv),
		env_(env),
		output_(&output)
	{
	}

	virtual char const *cgetenv(char const *key)
	{
		std::map<std::string,std::string>::const_iterator p = env_.find(key);
		if(p==env_.end())
			return "";
		return p->second.c_str();
	}
	virtual std::string getenv(std::string const &key)
	{
		std::map<std::string,std::string>::const_iterator p = env_.find(key);
		if(p==env_.end())
			return std::string();
		return p->second;
	}
	virtual std::map<std::string,std::string> const &getenv() 
	{
		return env_;
	}
	void async_read_headers(handler const &) 
	{
		throw std::runtime_error("dummy_api: unsupported");
	}

	void async_read_eof(callback const &) 
	{
		throw std::runtime_error("dummy_api: unsupported");
	}

	void async_write_some(void const *,size_t,io_handler const &)
	{
		throw std::runtime_error("dummy_api: unsupported");
	}
	virtual void write_eof(){}
	virtual size_t write_some(void const *p,size_t s,booster::system::error_code &) 
	{
		output_->append(reinterpret_cast<char const *>(p),s);
		return s;
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
	virtual void async_write_eof(handler const &)
	{
		throw std::runtime_error("dummy_api: unsupported");
	}
private:
	std::map<std::string,std::string> env_;
	std::string *output_;

};


#endif
