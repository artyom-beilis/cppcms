#include "application.h"
#include "url_dispatcher.h"
#include "applications_pool.h"
#include "service.h"
#include "http_response.h"
#include "http_request.h"
#include "http_cookie.h"
#include "locale_environment.h"
#include "http_context.h"
#include "format.h"
#include "aio_timer.h"
#include "intrusive_ptr.h"
#include <sstream>
#include <stdexcept>
#include <stdlib.h>

/*
class chat_room : public cppcms::application {
public:
	chat_room(cppcms::service &srv,int idle) : 
		cppcms::application(srv),
		timer_(srv),
		idle_time_(idle)
	{
		on_timeout(true);
		dispatcher().assign("^.*$",cppcms::util::mem_bind(&chat_room::json_call,self()));
	}
private:

	void on_error()
	{
		response().status(cppcms::http::response::bad_request);
		release_context(last_assigned_context());
		return;
	}
	void json_call()
	{
		using namespace cppcms;
		if(request().content_type()!="application/json") {
			on_error();
		}
		std::pair<void *,size_t> data=request().raw_post_data();
		std::istringstream ss(std::string((char *)data.first,data.second));
		json::value v;
		if(!v.load(ss,true)) {
			on_error();
			return;
		}
		if(	v.find("method").type()!=cppcms::json::is_string 
			|| v.find("params").type()!=json::is_array
			|| v.find("id").type()==json::is_undefined)
		{
			on_error();
			return;
		}
		std::string method=v.get<std::string>("method");
		json::array params=v.at("params").array();
		json::value id=v.at("id");
		json::value resp;
		resp.set("id",id);
		resp.set("result",json::null());
		resp.set("error",json::null());
		
		bool write=true;
		if(method=="submit")
			submit=(params,resp);
		else if(method=="fetch")
			write=fetch(params,resp);
		else {
			resp.set("error","Undefined Method "+method);
		}
		if(write) {
			response().content_type("application/json");
			response().out() << resp;
			release_context(last_assigned_context());
		}
		else {
			assign_context(0);
		}
	}
	
	void submit(json::array const &params,json::value &output)
	{
		if(params.size()!=1 || !params[0].type()==json::is_string) {
			output.set("error","Invalid input parameters");
			return;
		}
	}

	intrusive_ptr<chat_room> self()
	{
		return this;
	}
	void update_timer()
	{
		timer_.cancel();
	}
	void on_timeout(bool canceled)
	{
		if(canceled) {
			timer_.expires_from_now(idle);
			timer_.async_wait(cppcms::util::mem_bind(&stock::on_timeout,self()));
		}

		release_all_contexts();
		// And Die ;)
	}
};
*/

class stock : public cppcms::application {
public:
	stock(cppcms::service &srv) : cppcms::application(srv),timer_(srv)
	{
		dispatcher().assign("^/price$",&stock::get,this);
		dispatcher().assign("^/update$",&stock::update,this);
		price_=10.3;
		counter_=0;
		std::cout<<"stock()"<<std::endl;
		async_run();
	}
	~stock()
	{
		std::cout<<"~stock()"<<std::endl;
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
		dispatcher().assign(".*",&hello::hello_world,this);
		std::cout<<"hello()"<<std::endl;
	}
	~hello()
	{
		std::cout<<"~hello()"<<std::endl;
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
		std::ostringstream ss;
		ss<<time(NULL);
		response().set_cookie(cppcms::http::cookie("test",ss.str()));
		response().out() <<
			"<html><body>\n"
			"<h1>Hello World!</h1>\n";
		cppcms::http::request::cookies_type::const_iterator p;
		for(p=request().cookies().begin();p!=request().cookies().end();++p) {
			response().out()<<p->second<<"<br/>\n";
		}
		response().out()<<
			gt("hello\n") <<"<br>";
		for(int i=0;i<30;i++) {
			cppcms::util::format(response().out(),ngt("passed one day","passed %1% days",i),i) << "<br>";
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
