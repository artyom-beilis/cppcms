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
#include <cppcms/application.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <cppcms/http_cookie.h>
#include <cppcms/localization.h>
#include <cppcms/http_context.h>
#include <cppcms/filters.h>
#include <booster/intrusive_ptr.h>
#include <cppcms/form.h>
#include <cppcms/cache_interface.h>
#include <cppcms/session_interface.h>
#include <cppcms/mount_point.h>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <set>
#include <cppcms/config.h>
#include <cppcms/mem_bind.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/bind.hpp>
#else // Internal Boost
#   include <cppcms_boost/bind.hpp>
    namespace boost = cppcms_boost;
#endif
#include <fstream>

#include "hello_world_view.h"

#include <booster/aio/deadline_timer.h>
#include <booster/callback.h>
#include <booster/posix_time.h>
#include <booster/system_error.h>
#include <booster/log.h>


class chat : public cppcms::application {
public:
	chat(cppcms::service &srv) : cppcms::application(srv)
	{
		dispatcher().assign("^/post$",&chat::post,this);
		dispatcher().assign("^/get/(\\d+)$",&chat::get,this,1);
	}
	void post()
	{
		if(request().request_method()=="POST") {
			if(request().post().find("message")!=request().post().end()) {
				messages_.push_back(request().post().find("message")->second);
				broadcast();
			}
		}
		response().finalize();
		release_context()->async_complete_response();
	}
	void get(std::string no)
	{
		std::cerr<<"Get:"<<waiters_.size()<<std::endl;
		unsigned pos=atoi(no.c_str());
		if(pos < messages_.size()) {
			response().set_plain_text_header();
			response().out()<<messages_[pos];
			release_context()->async_complete_response();
		}
		else if(pos == messages_.size()) {
			booster::shared_ptr<cppcms::http::context> context=release_context();
			waiters_.insert(context);
			context->async_on_peer_reset(
				boost::bind(
					&chat::remove_context,
					booster::intrusive_ptr<chat>(this),
					context));
		}
		else {
			response().status(404);
			release_context()->async_complete_response();
		}
	}
	void remove_context(booster::shared_ptr<cppcms::http::context> context)
	{
		std::cerr<<"Connection closed"<<std::endl;
		waiters_.erase(context);
	}
	void broadcast()
	{
		for(waiters_type::iterator waiter=waiters_.begin();waiter!=waiters_.end();++waiter) {
			(*waiter)->response().set_plain_text_header();
			(*waiter)->response().out() << messages_.back();
			(*waiter)->async_complete_response();
		}
		waiters_.clear();
	}
private:
	std::vector<std::string> messages_;
	typedef std::set<booster::shared_ptr<cppcms::http::context> > waiters_type;
	waiters_type waiters_;
};


class stock : public cppcms::application {
public:
	stock(cppcms::service &srv) : cppcms::application(srv),timer_(srv.get_io_service())
	{
		dispatcher().assign("^/price$",&stock::get,this);
		dispatcher().assign("^/update$",&stock::update,this);
		price_=10.3;
		counter_=0;
		async_run();
	}
	~stock()
	{
	}
	void async_run()
	{
		on_timeout(booster::system::error_code());
	}
private:

	void on_timeout(booster::system::error_code const &/*e*/)
	{
		broadcast();
		timer_.expires_from_now(booster::ptime(100));
		timer_.async_wait(cppcms::util::mem_bind(&stock::on_timeout,booster::intrusive_ptr<stock>(this)));
	}
	void get()
	{
		response().set_plain_text_header();
		cppcms::http::request::form_type::const_iterator p=request().get().find("from"),
			e=request().get().end();
		if(p==e || atoi(p->second.c_str()) < counter_) {
			response().out() << price_<<std::endl;
			release_context()->async_complete_response();
			return;
		}
		all_.push_back(release_context());
	}
	void broadcast()
	{
		for(unsigned i=0;i<all_.size();i++) {
			all_[i]->response().out() << counter_<<":"<<price_ << std::endl;
			all_[i]->async_complete_response();
		}
		all_.clear();
	}
	void update()
	{
		if(request().request_method()=="POST") {
			cppcms::http::request::form_type::const_iterator p=request().post().find("price"),
				e=request().post().end();
			if(p!=e) {
				price_ = atof(p->second.c_str());
				counter_ ++ ;
				for(unsigned i=0;i<all_.size();i++) {
					all_[i]->response().out() << price_ << std::endl;
					all_[i]->async_complete_response();
				}
				all_.clear();
			}
		}

		response().out() <<
			"<html>"
			"<body><form action='/stock/update' method='post'> "
			"<input type='text' name='price' /><br/>"
			"<input type='submit' value='Update Price' name='submit' />"
			"</form></body></html>"<<std::endl;

		release_context()->async_complete_response();
	}

	int counter_;
	double price_;
	std::vector<booster::shared_ptr<cppcms::http::context> > all_;
	booster::aio::deadline_timer timer_;
};

class my_form : public cppcms::form
{
public:
	cppcms::widgets::text name;
	cppcms::widgets::numeric<double> age;
	cppcms::widgets::password p1;
	cppcms::widgets::password p2;
	cppcms::widgets::textarea description;
	cppcms::widgets::select_multiple sel;
	cppcms::widgets::radio sel1;
	cppcms::widgets::select sel2;
	cppcms::widgets::file gif;
	cppcms::widgets::file text;
	cppcms::widgets::hidden secret;
	
	my_form()
	{
		name.message("Your Name");
		name.limits(2,30);
		age.message("Your Age");
		age.range(0,120);
		description.message("Describe Yourself");
		p1.message("Password");
		p2.message("Confirm");
		p1.check_equal(p2);
		p1.non_empty();
		sel.message(cppcms::locale::translate("Files:"));
		sel.add("Foo");
		sel.add("Bar");
		sel.add("Bee");
		sel.add("Car");
		sel1.message(cppcms::locale::translate("Fruit"));
		sel1.add("Apple");
		sel1.add("Orange");
		sel2.message(cppcms::locale::translate("Sex"));
		sel2.add("Male");
		sel2.add("Femail");
		sel2.add("You are sexist");
		sel.at_least(2);
		sel1.non_empty();
		gif.non_empty();
		gif.add_valid_magic("GIF89a");
		gif.add_valid_magic("GIF87a");
		gif.add_valid_magic("\x89PNG\r\n\x1a\n");
		text.mime(booster::regex("text/.*"));
		*this + name + age + p1 + p2 + description + sel + sel1 + sel2 + gif + text + secret ;
	}
};

struct foo {
	void bar() {}
} foo_instance;


class hello : public cppcms::application {
public:
	hello(cppcms::service &srv) : 
		cppcms::application(srv)
	{
		dispatcher().assign("^/(\\d+)$",&hello::num,this,1);
		dispatcher().assign("^/view(/(\\w+))?$",&hello::view_test,this,2);
		dispatcher().assign("^/get$",&hello::gform,this);
		dispatcher().assign("^/env$",&hello::env,this);
		dispatcher().assign("^/post$",&hello::pform,this);
		dispatcher().assign("^/err$",&hello::err,this);
		dispatcher().assign("^/forward$",&hello::forward,this);
		dispatcher().assign("^/form$",&hello::form,this);
		dispatcher().assign("^/cache/?$",&hello::cached,this);
		dispatcher().assign("^/csrf/?$",&hello::csrf,this);
		dispatcher().assign("^/throw/?$",&hello::throw_it,this);
		dispatcher().assign("^/foo$",&foo::bar,&foo_instance);
		dispatcher().assign("^/session/?$",&hello::session_test,this);
		dispatcher().assign("^/verylong/?$",&hello::verylong,this);
		dispatcher().assign(".*",&hello::hello_world,this);
	}
	~hello()
	{
	}

	void env()
	{
		std::map<std::string,std::string> m(request().getenv());
		for(std::map<std::string,std::string>::const_iterator p = m.begin();p!=m.end();++p) {
			response().out() << p->first <<"="<<p->second <<"<br>\n";
		}
	}

	void csrf()
	{
		session().set("name","me");
		view::csrf c;
		if(request().request_method() == "POST") {
			c.my_form.load(context());
			if(c.my_form.validate()) {
				c.valid=true;
			}
		}
		render("csrf",c);
	}

	void verylong()
	{
		for(int i=0;i<10000000;i++) {
			if(!(response().out() << i << '\n')) {
				BOOSTER_DEBUG("hello") << "Bad output!";
				return;
			}
		}
		
	}

	void throw_it()
	{
		throw std::runtime_error("This is my secret error message");
	}

	void session_test()
	{
		if(!session().is_set("first_visit"))
			session().set("first_visit",::time(0));
		std::string tmp(rand() % 100,'x');
		session().set("foo",tmp);
		response().out() << cppcms::locale::format("<html><body> Your first visit was at {1,datetime=l} </body></html>") 
			% session().get<time_t>("first_visit");
	}

	void cached()
	{
		if(cache().fetch_page("test"))
			return;

		response().out() <<
			"<html><body>Time :" << cppcms::locale::format("{1,time=f}") % time(0) << "</body></html>\n";

		cache().store_page("test",10);
	}

	void view_test(std::string skin)
	{
		view::hello c;
		if(!skin.empty())
			context().skin(skin);
		render("hello",c);
	}
	#ifndef CPPCMS_DISABLE_ICU_LOCALIZATION

	void devide(cppcms::locale::boundary::boundary_type type,std::string const &str,char const *name)
	{
		response().out()<<name<<":";
		using namespace cppcms::locale::boundary;
		ssegment_index ind(type,str.begin(),str.end(),context().locale());

		ssegment_index::iterator p;

		for(p=ind.begin();p!=ind.end();++p) {
			response().out()<<"|"<<*p;
		}
		response().out()<<"|<br>\n";
	}

	#endif

	void form()
	{
		my_form f;
		bool ok=false;
		if(request().request_method()=="POST") {
			f.load(context());
			if(f.validate()) {
				ok=true;	
			}
		}
		response().out()<<
			"<html><body>\n";
	
		std::locale loc = context().locale();

		if(f.name.set()) {
			using namespace cppcms::locale;
			std::string name = f.name.value();
			response().out() <<"Upper: "<<to_upper(name,loc)<<"<br>"<<std::endl;
			response().out() <<"Lower: "<<to_lower(name,loc)<<"<br>"<<std::endl;
			#ifndef CPPCMS_DISABLE_ICU_LOCALIZATION
			response().out() <<"Title: "<<to_title(name,loc)<<"<br>"<<std::endl;
			response().out() <<"Fold Case: "<<fold_case(name,loc)<<"<br>"<<std::endl;
			#endif

		}
		if(f.description.set()) {
			std::string descr = f.description.value();
			if(!f.description.valid()) {
				std::ofstream tmp("test.txt");
				tmp<<descr;
			}
			#ifndef CPPCMS_DISABLE_ICU_LOCALIZATION
			using namespace cppcms::locale;
			devide(boundary::character,descr,"code");
			devide(boundary::word,descr,"word");
			devide(boundary::sentence,descr,"sentence");
			devide(boundary::line,descr,"line");
			#endif
		}

		if(ok) {
			response().out() << f.name.value() <<" " <<f.age.value();
			f.clear();
		}


		
		response().out()<<
			"<form action='" <<
				request().script_name() + request().path_info()
			<<  "' method='post' enctype='multipart/form-data' >\n"
			"<table>\n";

		cppcms::form_context context(response().out());
		f.render(context);

		response().out()<<"</table><input type='submit' value='Send' ></form>\n";
		response().out()<<"</form></body></html>"<<std::endl;
	}

	void forward()
	{
		//response().set_redirect_header("http://127.0.0.1:8080/hello");
		response().set_redirect_header("/hello");
	}
	void err()
	{
		throw std::runtime_error("Foo<Bar>!!!");
	}
	void hello_world()
	{
		using namespace cppcms;
		std::ostringstream ss;
		ss<<std::time(NULL);
		response().set_cookie(cppcms::http::cookie("test",ss.str()));
		response().out() <<
			"<html><body>\n"
			"<h1>Hello World!</h1>\n";
		std::string t1="text";
		std::string t2="text text";
		std::string t3="<text>";
		response().out()<<cppcms::filters::date(time(0)) << "<br>\n";
		response().out()<<cppcms::filters::time(time(0)) << "<br>\n";
		response().out()<<cppcms::filters::datetime(time(0)) << "<br>\n";
		response().out()<<cppcms::filters::to_upper(t1) << "<br>\n";
		response().out()<<cppcms::filters::to_lower(t1) << "<br>\n";
		response().out()<<cppcms::filters::to_title(t1) << "<br>\n";
		response().out()<<cppcms::filters::escape(t3) << "<br>\n";
		response().out()<<cppcms::filters::urlencode(t2) << "<br>\n";
		response().out()<<cppcms::filters::base64_urlencode(t1) << "<br>\n";
		response().out()<<locale::format("{1,datetime=f}") % time(0) << "<br>\n";
		response().out()<<locale::format("{1,datetime=f}") % cppcms::filters::escape(time(0)) << "<br>\n";
		response().out()<<std::use_facet<locale::info>(response().out().getloc()).language()<<"<br>\n";
		cppcms::http::request::cookies_type::const_iterator p;
		for(p=request().cookies().begin();p!=request().cookies().end();++p) {
			response().out()<<p->second<<"<br/>\n";
		}

		response().out() << filters::date(time(0)) <<std::endl;

		response().out() << filters::escape(locale::translate("hello\n")) << "<br>";
		
		for(int i=0;i<30;i++) {
			response().out() << locale::format(locale::translate("passed one day","passed {1} days",i)) % i << "<br>\n";
		}
		response().out()
			<<"<body></html>\n";

	}
	void init()
	{
		//std::cout << "There" << std::endl;
	}
	void num(std::string s)
	{
		response().set_plain_text_header();
		response().out() <<
			"Number is "<<s;
	}
	void pform()
	{
		std::string name;
		std::string message;
		cppcms::http::request::form_type::const_iterator p=request().post().find("name");
		if(p!=request().post().end())
			name=p->second;
		if((p=request().post().find("message"))!=request().post().end())
			message=p->second;
		
		response().out() <<
			"<html><body>"
			"<form method=\"post\" action=\"/hello/post\" >"
			"<input name=\"name\" type=\"text\" /><br/>"
			"<textarea name=\"message\"></textarea><br/>"
			"<input type=\"Submit\" value=\"Submit\" />"
			"</form>"
			<<"Name:"<<name<<"<br/>\n"
			<<"Text:"<<message<<
			"</body></html>";

	}
	void gform()
	{
		std::string name;
		std::string message;
		cppcms::http::request::form_type::const_iterator p=request().get().find("name");
		if(p!=request().get().end())
			name=p->second;
		if((p=request().get().find("message"))!=request().get().end())
			message=p->second;
		
		response().out() <<
			"<html><body>"
			"<form method=\"get\" action=\"/hello/get\" >"
			"<input name=\"name\" type=\"text\" /><br/>"
			"<textarea name=\"message\"></textarea><br/>"
			"<input type=\"Submit\" value=\"Submit\" />"
			"</form>"
			<<"Name:"<<name<<"<br/>\n"
			<<"Text:"<<message<<
			"</body></html>";

	}
};


int main(int argc,char **argv)
{
	try {
		cppcms::service service(argc,argv);
		booster::intrusive_ptr<chat> c=new chat(service);
		service.applications_pool().mount(c,cppcms::mount_point("/chat"));
		service.applications_pool().mount(cppcms::applications_factory<hello>(),cppcms::mount_point("/hello"));
		service.applications_pool().mount(new stock(service),cppcms::mount_point("/stock"));
		service.run();
		std::cout<<"Done..."<<std::endl;
	}
	catch(std::exception const &e) {
		std::cerr<<"Caught exception: "<<e.what()<<std::endl;
		std::cerr<<booster::trace(e) << std::endl;
		return 1;
	}
	return 0;
}
