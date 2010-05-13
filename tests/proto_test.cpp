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
#include <iostream>
#include "client.h"

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string /*unused*/)
	{
		response().set_plain_text_header();
		if(!is_asynchronous())
			response().io_mode(cppcms::http::response::nogzip);
		std::map<std::string,std::string> env=request().getenv();
		std::ostream &out = response().out();
		for(std::map<std::string,std::string>::const_iterator p=env.begin();p!=env.end();++p) {
			out << p->first <<':'<<p->second << '\n';
		}
		out << '\n';
		typedef cppcms::http::request::form_type form_type;
		form_type const &form=request().post();
		for(form_type::const_iterator p=form.begin();p!=form.end();++p) {
			out << p->first <<'='<<p->second << '\n';
		}
		if(is_asynchronous())
			release_context()->async_complete_response();
	}
};





int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		booster::intrusive_ptr<cppcms::application> app;
		if(!srv.settings().get("test.async",false)) {
			srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		}
		else {
			app=new unit_test(srv);
			srv.applications_pool().mount(app);
		}
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return run_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
