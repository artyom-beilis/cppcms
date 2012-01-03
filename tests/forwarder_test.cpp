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
#include <cppcms/http_context.h>
#include <cppcms/forwarder.h>
#include <cppcms/mount_point.h>
#include <cppcms/json.h>
#include <cppcms/forwarder.h>
#include <iostream>
#include "client.h"
#include "test.h"
#include <booster/shared_ptr.h>
#include <booster/thread.h>

class unit_test : public cppcms::application 
{
public:
	unit_test(cppcms::service &s) : cppcms::application(s) 
	{}

	virtual void main(std::string path)
	{
		bool flush=false;

		if(path=="/status")
			response().status(201);
		if(path=="/chunks")
			flush=true;

		if(flush)
			response().io_mode(cppcms::http::response::normal);
		response().out()<< "path=" << path << '\n' ;
		if(flush)
			response().out() << std::flush;

		response().out()<< "method=" << request().request_method() << '\n';
		if(flush)
			response().out() << std::flush;
		for(cppcms::http::request::form_type::const_iterator p=request().post().begin(),e=request().post().end();p!=e;++p){
			response().out() << p->first <<"=" << p->second << '\n';
			if(flush)
				response().out() << std::flush;
		}
	}
};

bool fw_ok=false;

class forwarder {
public:
	forwarder(bool internal)
	{
		cppcms::json::value settings;
		settings["service"]["api"]="http";
		settings["service"]["port"]=8080;
		settings["service"]["disable_global_exit_handling"]=true;
		settings["http"]["script_names"][0]="/test";
		if(internal) {
			settings["forwarding"]["rules"][0]["ip"]="127.0.0.1";
			settings["forwarding"]["rules"][0]["port"]=8081;
		}
		srv.reset(new cppcms::service(settings));

		if(!internal) {
			std::cout << "Tesing application level forwarding" << std::endl;
			app_=new mini_forwarder(*srv);
			srv->applications_pool().mount(app_);
		}
		else {
			std::cout << "Tesing internal forwarder" << std::endl;
			TEST(srv->forwarder().check_forwading_rules("127.0.0.1:8080","/test","").second==8081);
		}

	}

	booster::shared_ptr<cppcms::service> service()
	{
		return srv;
	}

	void operator()() const
	{
		try {
			srv->run();
		}
		catch(std::exception const &e) {
			std::cerr << "Forwarder error:" << e.what() << std::endl;
			fw_ok=false;
			return;
		}
		fw_ok=true;
	}
private:
	struct mini_forwarder : public cppcms::application {
		mini_forwarder(cppcms::service &s) : cppcms::application(s) {}
		virtual void main(std::string /*unused*/)
		{
			cppcms::forward_connection(release_context(),"127.0.0.1",8081);
		}
	};
	booster::shared_ptr<cppcms::service> srv;
	booster::intrusive_ptr<cppcms::application> app_;
};


int main(int argc,char **argv) 
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		if(srv.settings().find("test.exec").type()==cppcms::json::is_string)
			srv.after_fork(submitter(srv));
	
		bool internal=srv.settings().get<bool>("test.internal");	
		forwarder fw(internal);
		booster::thread fw_thread(fw);
		
		srv.run();
		fw.service()->shutdown();
		fw_thread.join();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	if(run_ok  && fw_ok) {
		std::cout << "Done Ok" << std::endl;
		return EXIT_SUCCESS;
	}
	else {
		std::cerr << "Failed run=" << run_ok << " forwarder=" << fw_ok << std::endl;
		return EXIT_FAILURE;
	}
}


