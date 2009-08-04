#include "application.h"
#include "url_dispatcher.h"
#include "applications_pool.h"
#include "service.h"
#include "http_response.h"
#include "http_request.h"
#include "http_cookie.h"
#include <sstream>
#include <stdexcept>


class hello : public cppcms::application {
public:
	hello(cppcms::service &srv) : 
		cppcms::application(srv)
	{
		dispatcher().assign("^/(\\d+)$",&hello::num,this,1);
		dispatcher().assign("^/get$",&hello::gform,this);
		dispatcher().assign("^/post$",&hello::pform,this);
		dispatcher().assign("^/err$",&hello::err,this);
		dispatcher().assign(".*",&hello::hello_world,this);
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
		response().out() <<
			"<body></html>\n";

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
		service.applications_pool().mount(".*",cppcms::applications_factory<hello>());
		service.run();
		std::cout<<"Done..."<<std::endl;
	}
	catch(std::exception const &e) {
		std::cerr<<"Catched exception: "<<e.what()<<std::endl;
		return 1;
	}
	return 0;
}
