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
#include <cppcms/service.h>
#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/http_request.h>
#include <cppcms/http_response.h>
#include <cppcms/http_context.h>
#include <cppcms/json.h>
#include <cppcms/forwarder.h>
#include <iostream>
#include "client.h"

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
	forwarder()
	{
		cppcms::json::value settings;
		settings["service"]["api"]="http";
		settings["service"]["port"]=8080;
		settings["http"]["script_names"][0]="/test";

		srv.reset(new cppcms::service(settings));
		app_=new mini_forwarder(*srv);
		srv->applications_pool().mount(app_);

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
		virtual void main(std::string unused)
		{
			cppcms::forward_connection(release_context(),"127.0.0.1",8081);
		}
	};
	booster::shared_ptr<cppcms::service> srv;
	booster::intrusive_ptr<cppcms::application> app_;
};


int main(int argc,char **argv) {
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		if(srv.settings().find("test.exec").type()==cppcms::json::is_string)
			srv.after_fork(submitter(srv));
		
		forwarder fw;
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


