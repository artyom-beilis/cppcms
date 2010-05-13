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
#include <cppcms/thread_pool.h>
#include <cppcms/localization.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/form.h>
#include <cppcms/http_response.h>
#include <iostream>

#include "client.h"

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	void load(cppcms::form &f)
	{
		if(request().request_method()=="POST" || !request().query_string().empty()) {
			response().set_plain_text_header();
			std::ostream &out = response().out();
			f.load(context());
			bool v=f.validate();
			if(v) {
				out << "valid\n";
			}
			else {
				out << "invalid\n";
			}
		}
	}
	virtual void main(std::string a_case)
	{
		cppcms::widgets::text text;
		cppcms::widgets::textarea textarea;
		cppcms::widgets::numeric<int> integer;
		cppcms::widgets::numeric<double> real;
		cppcms::widgets::password p1,p2;
		cppcms::widgets::regex_field regex("^(yes|no)$");
		cppcms::widgets::email email;
		cppcms::widgets::checkbox checkbox;
		cppcms::widgets::select_multiple select_multiple;
		cppcms::widgets::select select;
		cppcms::widgets::radio radio;
		cppcms::widgets::submit submit;


		text.message("text");
		textarea.message("textarea");
		integer.message("int");
		real.message("double");
		p1.message("pass");
		p2.message("pass2");
		regex.message("yes or not");
		email.message("E-Mail");
		checkbox.message("Checkbox");
		select_multiple.message("Select Multiple");
		select.message("Select");
		radio.message("Radio");
		submit.message("Submit");
		submit.value("Button");

		select_multiple.add("a",true);
		select_multiple.add("b",true);
		select_multiple.add("c");
		select_multiple.add(cppcms::locale::translate("tr1"),std::string("id1"));

		select.add("a");
		select.add("b");
		select.add("c");
		select.add(cppcms::locale::translate("tr2"),"id2");
		select.selected_id("id2");

		radio.add("x");
		radio.add("y");
		radio.add(cppcms::locale::translate("tr3"),"id3");
		radio.selected(0);

		cppcms::form X,a,a1,a2,b;

		if(a_case.empty() || a_case == "/non_empty" || a_case=="/sub"){
			X+text+a+b;
			a+a1;
			a+a2;
			a1+textarea;
			a2+integer;
			a2+real;
			b+p1+p2+regex+email+checkbox+select_multiple;
			X+select+radio+submit;

			if(!a_case.empty()) {
				text.non_empty();
				textarea.non_empty();
				integer.non_empty();
				real.non_empty();
				p1.non_empty();
				p2.non_empty();
				regex.non_empty();
				email.non_empty();
				select_multiple.non_empty();
				select.non_empty();
				radio.non_empty();
			}

			if(request().request_method()=="POST" || !request().query_string().empty()) {
				response().set_plain_text_header();
				std::ostream &out = response().out();
				X.load(context());
				if(X.validate()) {
					out << "valid" << std::endl;
				}
				else {
					out << "invalid" << std::endl;
				}
				if(text.set()) out << "Text:" << text.value() << std::endl;
				if(textarea.set()) out << "Textarea:" << textarea.value() << std::endl;
				if(integer.set()) out << "Int:" << integer.value() << std::endl;
				if(real.set()) out << "Double:" << real.value() << std::endl;
				if(p1.set()) out << "p1:" << p1.value() << std::endl;
				if(p2.set()) out << "p2:" << p2.value() << std::endl;
				if(regex.set()) out << "regex: " << regex.value() << std::endl;
				if(email.set()) out << "email: " << email.value() << std::endl;
				if(checkbox.set()) out << "checkbox: " << checkbox.value() << std::endl;
				if(select_multiple.set()) {
					out << "selected m:";
					std::vector<bool> v=select_multiple.selected_map();
					for(unsigned i=0;i<v.size();i++)
						out << i << " ";
					out << std::endl;
					std::set<std::string> ids=select_multiple.selected_ids();
					out << "selected m ids:";
					for(std::set<std::string>::const_iterator p=ids.begin();p!=ids.end();++p)
						out << *p << " ";
					out<< std::endl;
				}
				if(select.set()) 
					out << "Select: selected " << select.selected() << " id=" << select.selected_id() << std::endl;
				if(radio.set()) 
					out << "Radio: selected " << radio.selected() << " id=" << radio.selected_id() << std::endl;
				if(submit.set())
					out << "Submit pressed: " << submit.value() << std::endl;

			} 
			else if(a_case !="/sub") {
				std::ostream &out = response().out();
				out << "non loaded<br>\n";
				cppcms::form_context context(out);
				out << "<form action=\"" << request().script_name() + request().path_info() <<  "\" method=\"post\" >\n";
				X.render(context);
				out << "</form>\n";
			}
			else { // if (a_case=="/sub")
				cppcms::form_context context(response().out());
				b.render(context);
			}


		}
		else if(a_case == "/text") {
			X+text;
			text.limits(2,5);
			load(X);
			if(text.set()) {
				response().out() << text.value();
			}

		}
		else if(a_case == "/number") {
			X+real;
			real.range(5,100);
			load(X);
			if(real.set()) {
				response().out() << real.value();
			}

		}
		else if(a_case == "/pass") {
			X+p1+p2;
			p1.non_empty();
			p2.check_equal(p1);
			load(X);
		}
		else if(a_case == "/checkbox") {
			X+checkbox;
			load(X);
			response().out() << checkbox.value();
		}
		else if(a_case == "/sm") {
			X+select_multiple;
			select_multiple.non_empty();
			select_multiple.at_most(2);
			load(X);
			std::ostream &out = response().out();
			std::vector<bool> v=select_multiple.selected_map();
			for(unsigned i=0;i<v.size();i++)
				out << v[i] << " ";
			out << std::endl;
			std::set<std::string> ids=select_multiple.selected_ids();
			for(std::set<std::string>::const_iterator p=ids.begin();p!=ids.end();++p)
				out << *p << " ";
			out<< std::endl;
		}
		else if(a_case == "/select") {
			X+select;
			select.non_empty();
			load(X);
			response().out() << select.selected() <<" " << select.selected_id();
		}
		else if(a_case == "/radio") {
			X+radio;
			radio.non_empty();
			load(X);
			response().out() << radio.selected() <<" " << radio.selected_id();
		}
		else if(a_case == "/submit") {
			X+submit;
			load(X);
			response().out() << submit.value();
		}
		else if(a_case == "/submitl") {
			cppcms::widgets::submit s;
			s.id("submit_id");
			s.name("submit_name");
			s.help(cppcms::locale::translate("help"));
			s.error_message(cppcms::locale::translate("error"));
			s.message(cppcms::locale::translate("message"));
			s.valid(false);
			s.value(cppcms::locale::translate("test"));
			X+s;
			cppcms::form_context context(response().out());
			X.render(context);
		}
	}
};





int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return run_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
