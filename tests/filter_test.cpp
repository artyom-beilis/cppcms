///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/service.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/http_content_filter.h>
#include <cppcms/http_context.h>
#include <cppcms/http_file.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/mount_point.h>
#include <booster/aio/deadline_timer.h>
#include <booster/posix_time.h>
#include <cppcms/json.h>
#include <booster/thread.h>
#include <iostream>
#include "client.h"
#include <sstream>
#include <booster/log.h>
#include "test.h"

int g_fail;
int total_on_error;
#define TESTNT(x) do { if(x) break;  std::cerr << "FAIL: " #x " in line: " << __LINE__ << std::endl; g_fail = 1; return; } while(0)


class basic_test :  public cppcms::application {
public:
	basic_test(cppcms::service &srv) : cppcms::application(srv) {}
	std::string get_ref(std::string const &name)
	{
		int len = atoi(request().get("l_" + name).c_str());
		char c = request().get("f_" + name).c_str()[0];
		std::string r;
		r.reserve(len);
		for(int i=0;i<len;i++) {
			r += c;
			c++;
			if(c>'z')
				c='a';
		}
		return r;
	}
	
	void do_abort(int code)
	{
		int how=atoi(request().get("how").c_str());
		switch(how){
		case 3:
			response().setbuf(0);
		case 2:
			response().full_asynchronous_buffering(false);
		case 1:
			response().status(code);
			response().set_plain_text_header();
			response().out() << "at="<<request().get("abort");
		case 0:
			throw cppcms::http::abort_upload(code);
		}
	}

};

class file_test : public basic_test, public cppcms::http::multipart_filter {
public:
	file_test(cppcms::service &s) : basic_test(s)
	{
	}

	struct test_data {
		int on_new_file;
		int on_upload_progress;
		int on_data_ready;
		int on_end_of_content;
		int on_error;
		void write(std::ostream &out)
		{
			out <<
			"on_new_file="<<on_new_file<<"\n"
			"on_upload_progress="<<on_upload_progress<<"\n"
			"on_data_ready="<<on_data_ready<<"\n"
			"on_end_of_content="<<on_end_of_content<<"\n"
			;
		}
	};

	test_data *data()
	{
		return context().get_specific<test_data>();
	}

	void on_new_file(cppcms::http::file &input_file) 
	{
		data()->on_new_file++;
		TESTNT(input_file.size() == 0);
		std::string loc;
		if((loc=request().get("save_to_"+input_file.name()))!="") {
			std::cerr << "Saving to " << loc << std::endl;
			input_file.output_file(loc);
		}
		if(request().get("abort")=="on_new_file" && input_file.name() == request().get("at"))
			do_abort(502);
	}
	void on_upload_progress(cppcms::http::file &input_file)
	{
		data()->on_upload_progress++;
		TESTNT(input_file.size() > 0);
		std::string ref = get_ref(input_file.name());
		TESTNT(size_t(input_file.size()) << ref.size());
		std::ostringstream ss;
		input_file.data().seekg(0);
		ss<<input_file.data().rdbuf();
		std::string r = ss.str();
		TESTNT(r.size() == size_t(input_file.size()));
		//std::cerr << "\n\nRef=" << ref << std::endl;
		//std::cerr << "Res=" << r << "\n\n" << std::endl;
		TESTNT(ref.substr(0,r.size())==r);
		if(request().get("abort")=="on_upload_progress" && input_file.name() == request().get("at"))
			do_abort(503);
	}
	void on_data_ready(cppcms::http::file &input_file) {
		data()->on_data_ready++;
		std::string ref = get_ref(input_file.name());
		TESTNT(size_t(input_file.size()) == ref.size());
		std::ostringstream ss;
		input_file.data().seekg(0);
		ss<<input_file.data().rdbuf();
		std::string r = ss.str();
		TESTNT(r.size() == size_t(input_file.size()));
		TESTNT(ref==r);
		if(request().get("abort")=="on_data_ready" && input_file.name() == request().get("at"))
			do_abort(504);
	}
	void on_end_of_content(){
		data()->on_end_of_content++;
		TESTNT(request().get("fail")=="");
		if(request().get("abort")=="on_end_of_content")
			do_abort(505);

	}
	void on_error() {
		data()->on_error++;
		TESTNT(request().get("fail")=="1");
		total_on_error++;
	}
	
	void main(std::string path)
	{
		if(path=="/total_on_error") {
			response().out() << "total_on_error=" << total_on_error;
			total_on_error = 0;
			return;
		}
		if(path=="/no_content") {
			TESTNT(request().is_ready());
			response().out() << "no_content=1";
			return;
		}
		if(request().get("setbuf")!="") {
			request().setbuf(atoi(request().get("setbuf").c_str()));
		}

		TESTNT(request().is_ready() || context().get_specific<test_data>()==0);

		if(!request().is_ready()) {
			test_data *td = new test_data();
			context().reset_specific<test_data>(td);
			if(request().get("abort")=="on_headers_ready")
				do_abort(501);
			request().set_content_filter(*this);
			std::string cl_limit,mp_limit;
			if((cl_limit=request().get("cl_limit"))!="") 
				request().limits().content_length_limit(atoi(cl_limit.c_str()));
			if((mp_limit=request().get("mp_limit"))!="") 
				request().limits().multipart_form_data_limit(atoi(mp_limit.c_str()));
		}
		else {
			test_data *td = context().get_specific<test_data>();
			TESTNT(td);
			if(request().get("abort")=="") {
				TESTNT(td->on_error == 0);
				TESTNT(td->on_end_of_content == 1);
				TESTNT(td->on_new_file == td->on_data_ready);
				if(request().content_type_parsed().is_multipart_form_data())
					TESTNT(td->on_new_file == int(request().post().size() + request().files().size()));
				if(request().get("chunks")!="")
					TESTNT(td->on_upload_progress != 0);
			}
			td->write(response().out());
			response().out() <<
			"files=" << request().files().size() << "\n"
			"post="  << request().post().size() << "\n"
			"total=" << request().files().size() + request().post().size() << "\n"
			;

		}
				
	}
};


class raw_test : public basic_test, public cppcms::http::raw_content_filter {
public:
	raw_test(cppcms::service &s) : basic_test(s)
	{
	}

	struct test_data {
		int on_data_chunk;
		int on_end_of_content;
		int on_error;
		std::string content;
		test_data() : on_data_chunk(0), on_end_of_content(0), on_error(0) {}
		void write(std::ostream &out)
		{
			out <<
			"on_data_chunk="<<on_data_chunk<<"\n"
			"on_end_of_content="<<on_end_of_content<<"\n"
			;
		}
	};

	test_data *data()
	{
		return context().get_specific<test_data>();
	}

	void on_data_chunk(void const *ptr,size_t data_size)
	{
		if(request().get("abort")=="on_data_chunk" && atoi(request().get("at").c_str()) >= int(data()->content.size()))
			do_abort(502);
		data()->on_data_chunk++;
		data()->content.append(static_cast<char const *>(ptr),data_size);
	}

	void on_end_of_content(){
		data()->on_end_of_content++;
		TESTNT(request().get("fail")=="");
		if(request().get("abort")=="on_end_of_content")
			do_abort(503);

	}
	void on_error() {
		data()->on_error++;
		TESTNT(request().get("fail")=="1");
		total_on_error++;
	}
	
	void main(std::string path)
	{
		if(path=="/total_on_error") {
			response().out() << "total_on_error=" << total_on_error;
			total_on_error = 0;
			return;
		}
		if(path=="/no_content") {
			TESTNT(request().is_ready());
			response().out() << "no_content=1";
			return;
		}
		if(request().get("setbuf")!="") {
			request().setbuf(atoi(request().get("setbuf").c_str()));
		}

		TESTNT(request().is_ready() || context().get_specific<test_data>()==0);

		if(!request().is_ready()) {
			test_data *td = new test_data();
			context().reset_specific<test_data>(td);
			if(request().get("abort")=="on_headers_ready")
				do_abort(501);
			request().set_content_filter(*this);
			std::string cl_limit,mp_limit;
			if((cl_limit=request().get("cl_limit"))!="") 
				request().limits().content_length_limit(atoi(cl_limit.c_str()));
			if((mp_limit=request().get("mp_limit"))!="") 
				request().limits().multipart_form_data_limit(atoi(mp_limit.c_str()));
		}
		else {
			test_data *td = context().get_specific<test_data>();
			TESTNT(td);
			if(request().get("abort")=="") {
				TESTNT(td->on_error == 0);
				TESTNT(td->on_data_chunk >= 1);
				TESTNT(td->on_end_of_content == 1);
				if(request().get("chunks")!="")
					TESTNT(td->on_data_chunk > 1);
				if(request().get("l_1")!="")
					TESTNT(td->content == get_ref("1"));
			}
			TESTNT(request().content_length() > 0);
			TESTNT(request().raw_post_data().second == 0);
			td->write(response().out());

		}
				
	}
};



int main(int argc,char **argv)
{
	try {
		using cppcms::mount_point;
		cppcms::service srv(argc,argv);

		srv.applications_pool().mount(	cppcms::create_pool<file_test>(),
						mount_point("/upload"),
						cppcms::app::asynchronous | cppcms::app::content_filter);
		
		srv.applications_pool().mount(	cppcms::create_pool<raw_test>(),
						mount_point("/raw"),
						cppcms::app::asynchronous | cppcms::app::content_filter);

		
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << booster::trace(e) << std::endl;
		return EXIT_FAILURE;
	}
	if(run_ok && !g_fail) {
		std::cout << "Full Test: Ok" << std::endl;
		return EXIT_SUCCESS;
	}
	else {
		std::cout << "FAILED" << std::endl;
		return EXIT_FAILURE;
	}
}
