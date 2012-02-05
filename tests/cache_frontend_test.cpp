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

#include <cppcms/serialization.h>

struct mydata : public cppcms::serializable { 
	int x;
	int y;
	mydata(int a=0,int b=0) : x(a), y(b) {}
	void serialize(cppcms::archive &a)
	{
		a & x & y;
	}
};


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




	void test_objects()
	{
		std::cout << "- Data Object" << std::endl;
		set_context(false);

		{
			std::cout << "-- With Triggers Full API" << std::endl;
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d(1,2);
				std::set<std::string> t1,t2;
				t1.insert("k1");
				t2.insert("k2");
				cache().store_frame("foo","bar",t1);
				cache().store_data("dat",d,t2);
				TEST(tr.detach().size()==4);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d;
				std::string tmp;
				TEST(cache().fetch_frame("foo",tmp));
				TEST(cache().fetch_data("dat",d));
				TEST(d.x==1  && d.y==2);
				TEST(tmp=="bar");
				std::set<std::string> tg=tr.detach();
				TEST(tg.size()==4);
				TEST(tg.count("foo")==1);
				TEST(tg.count("dat")==1);
				TEST(tg.count("k1")==1);
				TEST(tg.count("k2")==1);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d;
				std::string tmp;
				TEST(cache().fetch_frame("foo",tmp,true));
				TEST(cache().fetch_data("dat",d,true));
				TEST(d.x==1  && d.y==2);
				TEST(tmp=="bar");
				TEST(tr.detach().size()==0);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d(4,5);
				cache().store_frame("foo","baz",-1);
				cache().store_data("dat",d,-1);
				TEST(tr.detach().size()==2);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d;
				std::string tmp;
				TEST(cache().fetch_frame("foo",tmp));
				TEST(cache().fetch_data("dat",d));
				TEST(d.x==4  && d.y==5);
				TEST(tmp=="baz");
				std::set<std::string> tg=tr.detach();
				TEST(tg.size()==2);
				TEST(tg.count("foo")==1);
				TEST(tg.count("dat")==1);
			}
		}
		cache().clear();
		{
			std::cout << "-- Without Triggers" << std::endl;
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d(1,2);
				cache().store_frame("foo","bar",std::set<std::string>(),-1,true);
				cache().store_data("dat",d,std::set<std::string>(),-1,true);
				TEST(tr.detach().size()==0);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d;
				std::string tmp;
				TEST(cache().fetch_frame("foo",tmp,true));
				TEST(cache().fetch_data("dat",d,true));
				TEST(d.x==1  && d.y==2);
				TEST(tmp=="bar");
				TEST(tr.detach().size()==0);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d;
				std::string tmp;
				TEST(cache().fetch_frame("foo",tmp));
				TEST(cache().fetch_data("dat",d));
				TEST(d.x==1  && d.y==2);
				TEST(tmp=="bar");
				TEST(tr.detach().size()==2);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d(4,5);
				cache().store_frame("foo","baz",-1,true);
				cache().store_data("dat",d,-1,true);
				TEST(tr.detach().size()==0);
			}
			{
				cache().reset();
				cppcms::triggers_recorder tr(cache());
				mydata d;
				std::string tmp;
				TEST(cache().fetch_frame("foo",tmp,true));
				TEST(cache().fetch_data("dat",d,true));
				TEST(d.x==4  && d.y==5);
				TEST(tmp=="baz");
				TEST(tr.detach().size()==0);
			}
		}
		cache().clear();
		{
			std::cout << "-- Timeouts" << std::endl;
			{
				cache().reset();
				mydata d1(1,2);
				mydata d2(4,5);
				cache().store_frame("foo1","baz1",std::set<std::string>(),2,true);
				cache().store_data("dat1",d1,std::set<std::string>(),2,true);
				cache().store_frame("foo2","baz2",2,true);
				cache().store_data("dat2",d2,2,true);
			}
			{
				cache().reset();
				mydata d1,d2;
				std::string t1,t2;
				TEST(cache().fetch_frame("foo1",t1,true));
				TEST(cache().fetch_frame("foo2",t2,true));
				TEST(cache().fetch_data("dat1",d1,true));
				TEST(cache().fetch_data("dat2",d2,true));
				TEST(d1.x==1  && d1.y==2);
				TEST(d2.x==4  && d2.y==5);
				TEST(t1=="baz1");
				TEST(t2=="baz2");
			}
			booster::ptime::millisleep(3000);
			{
				cache().reset();
				mydata d1,d2;
				std::string t1,t2;
				TEST(!cache().fetch_frame("foo1",t1,true));
				TEST(!cache().fetch_frame("foo2",t2,true));
				TEST(!cache().fetch_data("dat1",d1,true));
				TEST(!cache().fetch_data("dat2",d2,true));
			}
		}


	}


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
		app.test_objects();
	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}



