///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include "tc_test_content.h"
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/json.h>
#include <cppcms/cache_interface.h>
#include <cppcms/url_mapper.h>
#include "dummy_api.h"
#include "test.h"

#include <iomanip>
#include <sstream>

void compare_strings(std::string const &l,std::string const &r,int file_line)
{
	if(l==r) {
		//std::cerr << "[" << l << "] == [" << r << "]" << " at " << file_line << " OK !"  << std::endl;
		return;
	}
	/*
	size_t m = l.size();
	if(r.size() > m)  m = r.size();
	int line = 1;
	for(size_t i=0;i<m;i++) {
		std::string lstr = conv(l,i);
		std::string rstr = conv(r,i);
		if(lstr=="\\n")
			line++;
		std::cerr << std::setw(4) << line << " [" << lstr << '|' << rstr << "]   ";
		if(lstr!=rstr)
			std::cerr << "<----------" << std::endl;
		else
			std::cerr << std::endl;
	}*/
	std::cerr << "[" << l << "]!=[" << r << "]" << " at " << file_line << std::endl;
	throw std::runtime_error("Failed test");

}

#define TEQ(l,r) compare_strings(l,r,__LINE__)

class test_app : public cppcms::application {
public:
	test_app(cppcms::service &srv) : 
		cppcms::application(srv),
		srv_(srv)
	{
		mapper().assign("foo","/foo");
		mapper().assign("/");
		mapper().assign("/{1}");
		mapper().assign("/{1}/{2}");
	}
	~test_app()
	{
		release_context();
	}
	void set_context(bool mark_chunks=false,bool mark_eof=false)
	{
		std::map<std::string,std::string> env;
		env["HTTP_HOST"]="www.example.com";
		env["SCRIPT_NAME"]="/foo";
		env["PATH_INFO"]="/bar";
		env["REQUEST_METHOD"]="GET";
		env["HTTP_ACCEPT_ENCODING"]=="gzip";
		booster::shared_ptr<dummy_api> api(new dummy_api(srv_,env,output_,mark_chunks,mark_eof));
		booster::shared_ptr<cppcms::http::context> cnt(new cppcms::http::context(api));
		assign_context(cnt);
		output_.clear();
	}

	std::string str()
	{
		std::string result = output_;
		output_.clear();
		return result;
	}

	void test_buffer_size(bool async)
	{
		std::cout << "- Test setbuf/flush " << (async ? "async" : "sync")<< std::endl;
		set_context(true,true);
		if(async) {
			response().io_mode(cppcms::http::response::asynchronous);
		}
		else {
			response().io_mode(cppcms::http::response::nogzip);
		}
		response().full_asynchronous_buffering(false);
		response().out();
		response().setbuf(0);
		str();
		response().out() << "x";
		TEQ(str(),"[x]");
		response().out() << 123;
		TEQ(str(),"[123]");
		response().setbuf(4);
		response().out() << "abcdefg";
		TEQ(str(),"[abcdefg]");
		response().out() << 124;
		TEQ(str(),"");
		response().out() << std::flush;
		TEQ(str(),"[124]");
		response().out() << "xxx";
		TEQ(str(),"");
		response().setbuf(0);
		TEQ(str(),"[xxx]");
		if(async) {
			response().setbuf(4);
			std::cout<< "-- fully/partially buffered mode" << std::endl;
			response().full_asynchronous_buffering(true);
			response().out() << "12345678";
			TEQ(str(),"");
			response().out() << std::flush;
			TEQ(str(),"");
			response().full_asynchronous_buffering(false);
			TEQ(str(),"[12345678]");
			response().full_asynchronous_buffering(true);
			response().out() << "123";
			TEQ(str(),"");
			response().full_asynchronous_buffering(false);
			response().out() << std::flush;
			TEQ(str(),"[123]");
		}
		response().finalize();
		TEQ(str(),"[EOF]");
	}

private:
	std::string output_;
	cppcms::service &srv_;
};


int main()
{
	try {
		cppcms::json::value cfg;
		cfg["cache"]["backend"]="thread_shared";
		cfg["cache"]["limit"]=100;
		cppcms::service srv(cfg);
		test_app app(srv);

		app.test_buffer_size(false);
		app.test_buffer_size(true);

	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}



