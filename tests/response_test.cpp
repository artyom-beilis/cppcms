///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////

#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/json.h>
#include <cppcms/cache_interface.h>
#include <cppcms/url_mapper.h>
#include <cppcms/config.h>
#include <cppcms/view.h>
#include <string>
#include <vector>
#include <iomanip>
#include "dummy_api.h"
#include "test.h"

#ifndef CPPCMS_NO_GZIP
#include <zlib.h>
#endif

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

std::string remove_brakets(std::string const &in)
{
    std::string r;
    for(size_t i=0;i<in.size();i++)
        if(in[i]!='[' && in[i]!=']')
            r+= in[i];
    return r;
}

#define TEQ(l,r) compare_strings(l,r,__LINE__)
#define TEQC(l,r) compare_strings(remove_brakets(l),remove_brakets(r),__LINE__)

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
		env["HTTP_ACCEPT_ENCODING"]="gzip";
		booster::shared_ptr<dummy_api> api(new dummy_api(srv_,env,output_,mark_chunks,mark_eof));
		booster::shared_ptr<cppcms::http::context> cnt(new cppcms::http::context(api));
		assign_context(cnt);
		response().io_mode(cppcms::http::response::normal);
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
		TEQC(str(),"[123]");
		response().setbuf(4);
		response().out() << "abcdefg";
		TEQ(str(),"[abcdefg]");
		response().out() << 124;
		TEQ(str(),"");
		response().out() << std::flush;
		TEQ(str(),"[124]");
		response().out() << '0';
		TEQ(str(),"");
		response().out() << '1';
		TEQ(str(),"");
		response().out() << '2';
		TEQ(str(),"");
		response().out() << '3';
		TEQ(str(),"");
		response().out() << '4';
		TEQ(str(),"[01234]");
		response().out() << "xxx";
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
	void reset_context(bool async,bool gzip,bool set_cache)
	{
		set_context(false,false);
		if(async) {
			response().io_mode(cppcms::http::response::asynchronous);
			response().full_asynchronous_buffering(false);
		}
		else {
			if(!gzip)
				response().io_mode(cppcms::http::response::nogzip);
		}
		if(set_cache)
			cache().fetch_page("none");
		response().out();
		str();
	}
	void test_io_error(bool async,bool gzip,bool cached)
	{
		std::cout << "- Test I/O errors " << (async ? "async" : "sync")
		          << (gzip ? " gzip" : "") << (cached ? " cached" : "") << std::endl;
		reset_context(async,gzip,cached);
		response().setbuf(0);
		response().out() << "XXXXXXXXXXXXXXXX";
		TEST(response().out());
		response().out() << std::flush;
		TEST(response().out());
		#ifndef CPPCMS_NO_GZIP
		if(gzip) {
			zsinit();
			TEST(zstr() == "XXXXXXXXXXXXXXXX");
			zs_done = true;
			zsdone();
		}
		#endif
		output_="$$$ERROR$$$";
		if(gzip || cached) {
			response().out() << "x" << std::flush;
		}
		else {
			response().out() << "x";
		}
		TEST(!response().out());
		reset_context(async,gzip,cached);
		response().setbuf(1024);
		response().out() << "XXXXXXXXXXXXXXXX";
		TEST(response().out());
		response().out() << std::flush;
		TEST(response().out());
		output_="$$$ERROR$$$";
		int i;
		std::cout << "--- error on stream... " << std::flush;
		for(i=0;i<=100000;i++) {
			response().out() << std::setw(9) << i << '\n';
			if(!response().out())
				break;
		}
		TEST(i<100000);
		std::cout << "Detected after " << (i*10) << " bytes " << std::endl;
		if(async) {
			reset_context(async,gzip,cached);
			response().setbuf(1024);
			response().out() << "xx" << std::flush;
			std::cout << "--- block on stream... " << std::flush;
			output_="$$$BLOCK$$$";
			int i;
			for(i=0;i<=100000;i++) {
				response().out() << std::setw(9) << i << '\n';
				TEST(response().out());
				if(response().pending_blocked_output())
					break;
			}
			TEST(i<100000);
			std::cout << "Detected after " << (i*10) << " bytes " << std::endl;
		}
	}
#ifndef CPPCMS_NO_GZIP
	z_stream zs;
	bool zs_done;
	std::string total;
	std::string totalz;

	void zsinit()
	{
		memset(&zs,0,sizeof(zs));
		inflateInit2(&zs,15+16);
		zs_done = false;
		total.clear();
		totalz.clear();
	}
	void zsdone()
	{
		TEST(zs_done == true);
		inflateEnd(&zs);
	}
	std::string zstr()
	{
		std::vector<char> out(4096);
		std::string s = str();
		totalz += s;
		/*for(size_t i=0;i<totalz.size();i++) {
			std::cout << std::setw(2) <<std::hex<< unsigned((unsigned char)totalz[i]);
		}
		std::cout << "->[" << total <<"]" << std::endl;
		*/
		if(zs_done) {
			TEST(s.empty());
			return std::string();
		}
		if(s.empty()) {
			return std::string();
		}
		zs.avail_in = s.size();
		zs.next_in = (Bytef*)s.c_str();
		zs.avail_out = out.size();
		zs.next_out = (Bytef*)&out[0];
		int r = inflate(&zs,0);
		TEST(r==0 || r==Z_STREAM_END);
		std::string output;
		output.assign(&out[0],out.size() - zs.avail_out);
		if(r == Z_STREAM_END)
			zs_done = true;
		total += output;
		return output;

	}
	void test_gzipped(bool cached)
	{
		std::cout << "- Test gzip setbuf/flush " << (cached ? "cached" : "non cached")<< std::endl;
		set_context(false,false);
		if(cached) {
			cache().fetch_page("none");
		}
		response().out();
		std::string temp = str();
		TEST(temp.find("\r\n\r\n")==temp.size()-4);
		zsinit();
		response().out() << "message";
		TEQ(zstr(),"");
		response().out() << std::flush;
		TEQ(zstr(),"message");
		response().setbuf(0);
		response().out() << "ABCD" << std::flush;
		TEQ(zstr(),"ABCD");
		response().setbuf(2);
		response().out() << "XYZ" << std::flush;
		TEQ(zstr(),"XYZ");
		response().setbuf(1024);
		response().out() << "11111111111111111111111111111111111" << std::flush;
		TEQ(zstr(),"11111111111111111111111111111111111");
		response().out() << "x";
		response().finalize();
		TEQ(zstr(),"x");
		zsdone();
		std::string ztmp = totalz;
		std::string tmp = total;
		zsinit();
		output_ = ztmp;
		TEST(tmp == zstr());
		zsdone();
		if(cached) {
			cache().store_page("p");
			std::string ztmp2;
			TEST(cache().fetch_frame("_Z:p",ztmp2,true));
			TEST(ztmp2 == ztmp);
			cache().rise("p");
		}
	}
#endif

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
		//void test_io_error(bool async,bool gzip,bool cached)
		app.test_io_error(false,false,false);
		app.test_io_error(false,true,false);
		app.test_io_error(false,false,true);
		app.test_io_error(false,true,true);

		app.test_io_error(true,false,false);
		app.test_io_error(true,false,true);

#ifndef CPPCMS_NO_GZIP
		app.test_gzipped(false);
		app.test_gzipped(true);
#endif

	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}



