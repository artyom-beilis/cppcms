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
#ifndef CPPCMS_IMPL_CGI_API_H
#define CPPCMS_IMPL_CGI_API_H

#include "noncopyable.h"
#include "refcounted.h"
#include "intrusive_ptr.h"
#include <vector>
#include <map>
#include "function.h"
#include "config.h"

#include <booster/system_error.h>
namespace booster { namespace aio { class io_service; }}


namespace cppcms {
	class service;
	class application;
	namespace http {
		class request;
		class response;
	}


namespace impl {
namespace cgi {

	typedef function<void(booster::system::error_code const &e)> handler;
	typedef function<void(booster::system::error_code const &e,size_t)> io_handler;
	typedef function<void()> callback;
	typedef function<void(bool)> ehandler;

	class acceptor : public util::noncopyable {
	public:
		virtual void async_accept() = 0;
		virtual void stop() = 0;
		virtual ~acceptor(){}
	};

	class connection : public refcounted
	{
	public:
		connection(cppcms::service &srv);
		virtual ~connection();
		cppcms::service &service();
	
		void async_prepare_request(	http::request &request,
						ehandler const &on_post_data_ready);

		void async_write_response(	http::response &response,
						bool complete_response,
						ehandler const &on_response_written);

		void async_complete_response(	ehandler const &on_response_complete);
		
		void aync_wait_for_close_by_peer(callback const &on_eof);

		virtual std::string getenv(std::string const &key) = 0;
		virtual std::map<std::string,std::string> const &getenv() = 0;
		size_t write(void const *data,size_t n);
		bool is_reuseable();
	
		std::string last_error();
	
	protected:

		/****************************************************************************/

		// These are abstract member function that should be implemented by
		// actual protocol like FCGI, SCGI, HTTP or CGI

		virtual void async_read_headers(handler const &h) = 0;
		virtual bool keep_alive() = 0;
		virtual void close() = 0;

		// Concept implementation headers		
		
		virtual void async_read_some(void *,size_t,io_handler const &h) = 0;
		virtual void async_read_eof(callback const &h) = 0;
		virtual void async_write_some(void const *,size_t,io_handler const &h) = 0;
		virtual void async_write_eof(handler const &h) = 0;
		virtual size_t write_some(void const *,size_t) = 0;
		virtual booster::aio::io_service &get_io_service() = 0;

		/****************************************************************************/

	protected:
		intrusive_ptr<connection> self();
		void async_read(void *,size_t,io_handler const &h);
		void async_write(void const *,size_t,io_handler const &h);
	private:

		struct reader;
		struct writer;

		friend struct reader;
		friend struct writer;
		void set_error(ehandler const &h,std::string s);
		void load_content(booster::system::error_code const &e,http::request *request,ehandler const &h);
		void on_post_data_loaded(booster::system::error_code const &e,http::request *r,ehandler const &h);
		void on_async_write_written(booster::system::error_code const &e,bool complete_response,ehandler const &h);
		void on_eof_written(booster::system::error_code const &e,ehandler const &h);
		void handle_eof(callback const &on_eof);

		std::vector<char> content_;
		cppcms::service *service_;
		std::string async_chunk_;
		std::string error_;
		bool request_in_progress_;

	};


} // cgi
} // impl
} // cppcms

#endif
