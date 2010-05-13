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
#include "rpc_json.h" 
#include "http_context.h"
#include "http_request.h"
#include "http_response.h"

#include <sstream>
#include <fstream>
#include <streambuf>

namespace cppcms {
namespace rpc {

    call_error::call_error(std::string const &m) : cppcms_error(m) {} 
	struct json_call::data {};

	json_call::json_call(http::context &c) 
	{
		if(c.request().content_type()!="application/json")
			throw call_error("Invalid content type"); 
		if(c.request().request_method()!="POST")
			throw call_error("Invalid request method"); 
		std::pair<void *,size_t> post_data = c.request().raw_post_data();
		std::istringstream ss(std::string(reinterpret_cast<char const *>(post_data.first),post_data.second));
		json::value request;
		if(!request.load(ss,true))
			throw call_error("Invalid JSON");
		if(	request.type("method")!=json::is_string 
			|| request.type("params")!=json::is_array
			|| request.type("id")==json::is_undefined)
		{
			throw call_error("Invalid JSON-RPC");
		}
		if(request.type("id")==json::is_null) {
			notification_ = true;
		}
		else {
			notification_ = false;
			id_.swap(request["id"]);
			params_.swap(request["params"].array());
			method_ = request.get<std::string>("method");
		}
	}

	json_call::~json_call()
	{
	}
	
	void json_call::return_result(http::context &c,json::value const &result)
	{
		c.response().set_content_header("application/json");
		c.response().out() <<
			"{\"id\":"<<id_<<",\"error\":null,\"result\":"<<result<<"}";
	}
	void json_call::return_error(http::context &c,json::value const &error)
	{
		c.response().set_content_header("application/json");
		c.response().out() <<
			"{\"id\":"<<id_<<",\"error\":"<<error<<",\"result\":null}";
	}

	http::context &json_call::context()
	{
		if(!context_.get())
			throw cppcms_error("No context assigned to rpc::json_call");
		return *context_;
	}
	void json_call::return_result(json::value const &v)
	{
		check_not_notification();
		return_result(context(),v);
		context().response().finalize();
		context().async_complete_response();
	}
	void json_call::return_error(json::value const &e)
	{
		check_not_notification();
		return_error(context(),e);
		context().response().finalize();
		context().async_complete_response();
	}

	void json_call::check_not_notification()
	{
		if(notification())
			throw call_error("Notification method should not return response");
	}

	std::string json_call::method()
	{
		return method_;
	}
	bool json_call::notification()
	{
		return notification_;
	}
	json::array const &json_call::params()
	{
		return params_;
	}

	void json_call::attach_context(booster::shared_ptr<http::context> c)
	{
		context_ = c;
	}
	
	struct json_rpc_server::data{};

	json_rpc_server::json_rpc_server(cppcms::service &srv) :
		application(srv)
	{	
	}
	json_rpc_server::~json_rpc_server()
	{
	}
	void json_rpc_server::smd(json::value const &v)
	{
		std::ostringstream ss;
		ss<<v;
		smd_=ss.str();
	}
	void json_rpc_server::smd_raw(std::string const &v)
	{
		smd_=v;
	}
	void json_rpc_server::smd_from_file(std::string const &file)
	{
		std::ifstream smd(file.c_str());
		if(!smd)
			throw cppcms_error("Failed to open:" + file);
		smd_.reserve(1024);
		smd_.assign(	std::istreambuf_iterator<char>(smd),
				std::istreambuf_iterator<char>());
	}

	void json_rpc_server::main(std::string /*unused*/)
	{
		if(!smd_.empty() && request().request_method()=="GET") {

			response().set_content_header("application/json");
			response().out() << smd_;

			if(is_asynchronous())
				release_context()->async_complete_response();
			return;
		}

		try {
			current_call_.reset(new json_call(context()));
			methods_map_type::iterator p=methods_.find(method());
			if(p==methods_.end()) {
				if(!notification())
					return_error("Method not found");
				return;
			}
			if(p->second.role == notification_role && !notification()) {
				return_error("The request should be notification");
				return;
			}
			if(p->second.role == method_role && notification()) {
				// No way to respond according to protocol
				return;
			}
			try {
				p->second.method(params());
			}
			catch(json::bad_value_cast const &e) {
				if(current_call_.get() && !notification())
					return_error("Invalid parameters");
				return;
			}
			catch(call_error const &e) {
				if(current_call_.get() && !notification())
					return_error(e.what());
				return;
			}
			catch(std::exception const &e) {
				if(current_call_.get() && !notification())
					return_error("Internal Service Error");
				return;
			}
		}
		catch(call_error const &e)
		{
			response().set_content_header("text/plain");
			response().out()<< e.what() << std::endl;
		}
	}

	void json_rpc_server::bind(std::string const &name,method_type const &method,role_type role)
	{
		method_data data;
		data.method=method;
		data.role=role;
		methods_[name]=data;
	}
	bool json_rpc_server::notification()
	{
		check_call();
		return current_call_->notification();
	}
	json::array const &json_rpc_server::params()
	{
		check_call();
		return current_call_->params();
	}
	void json_rpc_server::return_result(json::value const &r)
	{
		check_call();
		current_call_->return_result(context(),r);
	}
	void json_rpc_server::return_error(json::value const &e)
	{
		check_call();
		current_call_->return_error(context(),e);
	}
	booster::shared_ptr<json_call> json_rpc_server::release_call()
	{
		check_call();
		current_call_->attach_context(release_context());
		booster::shared_ptr<json_call> call = current_call_;
		current_call_.reset();
		return call;
	}
	std::string json_rpc_server::method()
	{
		check_call();
		return current_call_->method();
	}
	void json_rpc_server::check_call()
	{
		if(current_call_.get()==0)
			throw cppcms_error("JSON-RPC Request is not assigned to class");
	}


} // rpc
} // cppcms
