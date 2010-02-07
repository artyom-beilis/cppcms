#include <service.h>
#include <application.h>
#include <applications_pool.h>
#include <localization.h>
#include <url_dispatcher.h>
#include <form.h>
#include <http_response.h>
#include <iostream>

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string a_case)
	{
		cppcms::widgets::text text;
		cppcms::widgets::textarea textarea;
		cppcms::widgets::numeric<int> integer;
		cppcms::widgets::numeric<double> real;
		cppcms::widgets::password p1,p2;
		cppcms::util::regex e("^(yes|no)$");
		cppcms::widgets::regex_field regex(e);
		cppcms::widgets::email email;
		cppcms::widgets::checkbox checkbox;
		cppcms::widgets::select_multiple select_multiple;
		cppcms::widgets::select select;
		cppcms::widgets::radio radio;
		cppcms::widgets::submit submit;

		cppcms::form X,a,a1,a2,b;
		X+text+a+b;
		a+a1;
		a+a2;
		a1+textarea;
		a2+integer;
		a2+real;
		b+p1+p2+regex+email+checkbox+select_multiple;
		X+select+radio+submit;


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

		select.add("a");
		select.add("b");
		select.add("c");

		radio.add("x");
		radio.add("y");


		if(a_case == "" )
			;// Nothing checked
		else if(a_case == "non_empty") {
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

		std::ostream &out = response().out();

		if(request().request_method()=="POST" || !request().query_string().empty()) {
			response().set_plain_text_header();
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

		else {
			out << "non loaded<br>\n";
			cppcms::form_context context(out);
			out << "<form action=\"" << request().script_name() + request().path_info() <<  "\" method=\"post\" >\n";
			X.render(context);
			out << "</form>";
		}

	}
};





int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
