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
#include <booster/posix_time.h>
#include "dummy_api.h"
#include "test.h"

#include <iomanip>
#include <sstream>

class test_app : public cppcms::application {
public:
	test_app(cppcms::service &srv) : 
		cppcms::application(srv),
		srv_(srv)
	{
	}
	~test_app()
	{
		release_context();
	}
	void set_context(bool gzip=false)
	{
		std::map<std::string,std::string> env;
		env["HTTP_HOST"]="www.example.com";
		env["SCRIPT_NAME"]="/foo";
		env["PATH_INFO"]="/bar";
		env["REQUEST_METHOD"]="GET";
		if(gzip)
			env["HTTP_ACCEPT_ENCODING"]="gzip, deflate";
		booster::shared_ptr<dummy_api> api(new dummy_api(srv_,env,output_));
		booster::shared_ptr<cppcms::http::context> cnt(new cppcms::http::context(api));
		assign_context(cnt);
		output_.clear();
	}

	std::string str()
	{
		response().finalize();
		size_t from = output_.find("\r\n\r\n");
		std::string result;
		if(from==std::string::npos) {
			result = output_;
			gzip_ = false;
		}
		else {
			result = output_.substr(from+4);
			gzip_ = output_.substr(0,from).find("gzip")!=std::string::npos;
		}
		output_.clear();
		return result;
	}
	void test_basic()
	{
		std::cout << "- Page Basic" << std::endl;
		
		set_context(false);
		TEST(cache().has_cache());
		TEST(!cache().fetch_page("test"));
		response().out() << "test";
		cache().add_trigger("x");
		cache().store_page("test");
		TEST(str()=="test");
		
		set_context(false);
		TEST(cache_size()==1);
		TEST(cache().fetch_page("test"));
		TEST(str()=="test");
		
		set_context(false);
		cache().rise("x");
		TEST(cache_size() == 0);
		TEST(!cache().fetch_page("test"));
		cache().add_trigger("x");
		response().out() << "test2";
		cache().reset(); // reset works
		cache().store_page("test",2);
		TEST(str()=="test2");
		set_context(false);
		cache().rise("x");
		TEST(cache_size() == 1);
		TEST(cache().fetch_page("test"));
		TEST(str() == "test2");
		booster::ptime::millisleep(3000);
		set_context(false);
		TEST(cache().fetch_page("test")==false);
		cache().clear();
		release_context();
	}

	void test_gzip()
	{
		std::cout << "- Page Gzip" << std::endl;
		set_context(true);
		cache().clear();
		TEST(request().getenv("HTTP_ACCEPT_ENCODING") == "gzip, deflate");
		cache().fetch_page("test");
		response().out() << "gzip";
		cache().store_page("test");
		TEST(str().substr(0,2)=="\x1f\x8b");
		TEST(gzip_);
		
		set_context(false);
	
		TEST(request().getenv("HTTP_ACCEPT_ENCODING") == "");
		TEST(cache_size() == 1);

		TEST(cache().fetch_page("test") ==false);
		response().out() << "gzip";
		cache().store_page("test");
		TEST(str() == "gzip");
		TEST(!gzip_);

		set_context(false);
		TEST(cache().fetch_page("test"));
		TEST(str()=="gzip");
		TEST(!gzip_);

		set_context(true);
		TEST(cache().fetch_page("test"));
		TEST(str().substr(0,2)=="\x1f\x8b");
		TEST(gzip_);

		set_context(false);
		TEST(cache_size()==2);
		cache().clear();
		TEST(cache_size()==0);
		release_context();
	}

	/*

	void test_objects()
	{
		set_context(false);

		{
			cppcms::triggers_recorder tr(cache());
			cache().store_frame("foo","bar",true);
			TEST(tr.detch().size()==0);
		}
		{
			cppcms::triggers_recorder tr(cache());
			cache().store_frame("foo","bar",true);
			TEST(tr.detch().size()==0);
		}

	}

	*/

	unsigned cache_size()
	{
		unsigned keys,triggers;
		TEST(cache().stats(keys,triggers));
		return keys;
	}
	
private:
	std::string output_;
	bool gzip_;
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

		app.set_context();

		app.test_basic();
		app.test_gzip();
	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}



