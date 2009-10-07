#include "application.h"
#include "url_dispatcher.h"
#include "applications_pool.h"
#include "service.h"
#include "http_response.h"
#include "http_request.h"
#include "http_cookie.h"
#include "locale_environment.h"
#include "locale_charset.h"
#include "http_context.h"
#include "filters.h"
#include "aio_timer.h"
#include "intrusive_ptr.h"
#include "form.h"
#include "locale_convert.h"
#include "utf8_iterator.h"
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <set>
#include <boost/bind.hpp>
#include <fstream>


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
			cppcms::intrusive_ptr<cppcms::http::context> context=release_context();
			waiters_.insert(context);
			context->async_on_peer_reset(
				boost::bind(
					&chat::remove_context,
					cppcms::intrusive_ptr<chat>(this),
					context));
		}
		else {
			response().status(404);
			release_context()->async_complete_response();
		}
	}
	void remove_context(cppcms::intrusive_ptr<cppcms::http::context> context)
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
	typedef std::set<cppcms::intrusive_ptr<cppcms::http::context> > waiters_type;
	waiters_type waiters_;
};


class stock : public cppcms::application {
public:
	stock(cppcms::service &srv) : cppcms::application(srv),timer_(srv)
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
		on_timeout(false);
	}
private:

	void on_timeout(bool x)
	{
		broadcast();
		timer_.expires_from_now(100);
		timer_.async_wait(cppcms::util::mem_bind(&stock::on_timeout,cppcms::intrusive_ptr<stock>(this)));
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
	std::vector<cppcms::intrusive_ptr<cppcms::http::context> > all_;
	cppcms::aio::timer timer_;
};

class my_form : public cppcms::form
{
public:
	cppcms::widgets::text name;
	cppcms::widgets::number<double> age;
	cppcms::widgets::password p1;
	cppcms::widgets::password p2;
	cppcms::widgets::textarea description;
	
	my_form() :
		name("name","Your Name"),
		age("age","Your Age"),
		//p1("p1","Password"),
		//p2("p2","Confirm"),
		description("descr","Describe")
	{
		name.limits(2,30);
		age.range(0,120);
		//p1.check_equal(p2);
		//p1.non_empty();
		*this + name + age + /*p1 + p2 + */ description;
	}
};



class hello : public cppcms::application {
public:
	hello(cppcms::service &srv) : 
		cppcms::application(srv)
	{
		dispatcher().assign("^/(\\d+)$",&hello::num,this,1);
		dispatcher().assign("^/get$",&hello::gform,this);
		dispatcher().assign("^/post$",&hello::pform,this);
		dispatcher().assign("^/err$",&hello::err,this);
		dispatcher().assign("^/forward$",&hello::forward,this);
		dispatcher().assign("^/form$",&hello::form,this);
		dispatcher().assign(".*",&hello::hello_world,this);
	}
	~hello()
	{
	}

	template<typename iterator> 
	void devide(std::string const &str,char const *name)
	{
		response().out()<<name<<":";
		iterator it(str,locale().get());
		iterator prev = it;
		++it;
		response().out()<<std::string(*prev,*it);
		while(*it!=str.end()) {
			prev=it;
			++it;
			response().out()<<" | "<<std::string(*prev,*it);
		}
		response().out()<<"<br>\n"<<name<<" reversed:";
		it=str.end();
		std::string::const_iterator p=*it;
		--it;
		response().out()<<std::string(*it,p);
		while(*it!=str.begin()) {
			p=*it;
			--it;
			response().out()<<" | "<<std::string(*it,p);
		}
		response().out()<<"<br>\nRandom "<<name<<":";
		it = str.begin() + rand() % str.size();
		prev=it;
		it+=5;
		prev-=5;
		response().out()<<std::string(*prev,*it)<<"<br>\n";
	}

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
	
		cppcms::locale::convert const &cv=std::use_facet<cppcms::locale::convert>(locale().get());

		if(f.name.set()) {
			std::string name = f.name.value();
			response().out() <<"Upper: "<<cv.to_upper(name)<<"<br>"<<std::endl;
			response().out() <<"Lower: "<<cv.to_lower(name)<<"<br>"<<std::endl;
			response().out() <<"Title: "<<cv.to_title(name)<<"<br>"<<std::endl;
			response().out() <<"Normal: "<<cv.to_normal(name)<<"<br>"<<std::endl;

		}
		if(f.description.set()) {
			std::string descr = f.description.value();
			if(!f.description.valid()) {
				std::ofstream tmp("test.txt");
				tmp<<descr;
			}
			
			devide<cppcms::utf8::const_code_point_iterator>(descr,"code");
			devide<cppcms::utf8::const_character_iterator>(descr,"char");
			devide<cppcms::utf8::const_word_iterator>(descr,"word");
			devide<cppcms::utf8::const_sentence_iterator>(descr,"sentence");
			devide<cppcms::utf8::const_line_iterator>(descr,"line");
		}

		if(ok) {

			response().out() << f.name.value() <<" " <<f.age.value();

			f.clear();
		}


		
		response().out()<<
			"<form action='" <<
				request().script_name() + request().path_info()
			<<  "' method='post'>\n"
			"<table>\n";

		f.render(response().out(),cppcms::form::as_table);

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
		cppcms::http::request::cookies_type::const_iterator p;
		for(p=request().cookies().begin();p!=request().cookies().end();++p) {
			response().out()<<p->second<<"<br/>\n";
		}

		time_t t=::time(NULL);
		std::tm tt;
		localtime_r(&t,&tt);

		response().out() << filters::date(tt) <<std::endl;

		response().out() << filters::escape(gt("hello\n")) << "<br>";
		
		for(int i=0;i<30;i++) {
			response().out() << format("To be or not to be %1%\n<br>",10);

			response().out() << format(ngt("passed one day","passed %1% days",i),i) << "<br>\n";
			//cppcms::util::format(response().out(),ngt("passed one day","passed %1% days",i),i) << "<br>";
		}
		response().out()
			<<"<body></html>\n";

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
		cppcms::intrusive_ptr<chat> c=new chat(service);
		service.applications_pool().mount(c,"/chat");
		service.applications_pool().mount(cppcms::applications_factory<hello>(),"/hello");
		service.applications_pool().mount(new stock(service),"/stock");
		service.run();
		std::cout<<"Done..."<<std::endl;
	}
	catch(std::exception const &e) {
		std::cerr<<"Catched exception: "<<e.what()<<std::endl;
		return 1;
	}
	return 0;
}
