///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/view.h>
#include <list>
class my_hello_world;
namespace view {

struct master : public cppcms::base_content {
	std::string title;
	bool ok;
};

struct data {
	std::string name;
	int val;
	data(char const *n="",int v=0) : name(n),val(v){}
};

struct a_form : public cppcms::form {
	cppcms::widgets::text username;
	cppcms::widgets::textarea name;
	cppcms::widgets::email mail;
	cppcms::widgets::password p1;
	cppcms::widgets::password p2;
	cppcms::widgets::numeric<int> integer;
	cppcms::widgets::numeric<double> real;
	cppcms::widgets::checkbox ok;
	//widgets::select fruit;
	//widgets::radio fruit;
	//widgets::select_multiple meat;
	//widgetset my_set;
	a_form() 
	{
		using cppcms::locale::translate;
		username.message(translate("Username"));
		name.message(translate("Real Name"));
		mail.message(translate("E-Mail"));
		p1.message(translate("Password"));
		p2.message(translate("Confirm"));
		integer.message(translate("Integer"));
		real.message(translate("Real"));
		ok.message(translate("Never Save"));
		add(username);
		add(mail);
		add(name);
		add(p1);
		add(p2);
		add(integer);
		add(real);
		add(ok);
		username.non_empty();
		name.non_empty();
		p2.check_equal(p1);
		p2.help(translate("(Same as above)"));
		p1.non_empty();
		p2.non_empty();
		real.range(-1.0,1.5);
/*		fruit.add("Orange");
		fruit.add("Palm");
		meat.add("Beef");
		meat.add("<<Chicken>>");
		meat.add("Duck");
		meat.set_min(2);
		meat.help=w->gettext("At least two choises");*/
	}
};


struct hello : public master {
	std::string username,realname,password;
	bool ok;
	std::string msg;
	typedef std::list<int> numbers_type;
	numbers_type numbers;
	
	std::list<data> lst;
	a_form form;
};

struct csrf : public cppcms::base_content {
	cppcms::widgets::text text;
	cppcms::widgets::submit button;
	cppcms::form my_form;
	bool valid;
	csrf() {
		valid = false;
		my_form.add(text);
		my_form.add(button);
	}
};


};

