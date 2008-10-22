#ifndef CPPCMS_FORM_H
#define CPPCMS_FORM_H 
#include <string>
#include <list>
#include <cgicc/Cgicc.h>
#include <boost/regex.hpp>
#include <ostream>

namespace cppcms {
using namespace std;
class base_form {
protected:
	list<base_form *> elements;
public:
	virtual ~base_form();
	enum {	as_html = 0,
		as_xhtml= 1,
		as_p	= 0,
		as_table= 2,
		as_ul 	= 4,
		as_mask = 6,
		error_with   = 0,
		error_no = 0x10,
		error_only = 0x20,
		error_mask = 0x30, };

	virtual string render(int how);
	virtual void load(cgicc::Cgicc const &cgi);
	virtual bool validate();
	void append(base_form *ptr);
	inline base_form &operator & (base_form &f) { append(&f); return *this; }
};

namespace widgets {

class base_widget : public base_form {
public:
	bool is_set;
	base_widget(string name,string msg="");
	string id;
	string name;
	string msg;
	string error_msg;
	bool is_valid;
	virtual string render(int how);
	virtual string render_input(int ) = 0;
	virtual string render_error();
};

class text : public base_widget {
protected:
	string type;
	unsigned low;
	int high;
public:
	string value;
	text(string id,string msg="") : base_widget(id,msg) , type("text"){ low=0,high=-1; };
	void set_limits(int min,int max) { low=min,high=max; }
	void set_nonempty() { low = 1, high=-1;};
	void set_limits(string e,int min,int max) { error_msg=e; set_limits(min,max); }
	void set_nonempty(string e){ error_msg=e; set_nonempty(); };
	virtual string render_input(int how);
	void set(string const &s);
	string const &get();
	virtual bool validate();
	virtual void load(cgicc::Cgicc const &cgi);
};

class password: public text {
	password *other;
public:
	password(string id,string msg="") : text(id,msg) { type="password"; other=NULL; } ;
	void set_equal(password &p2) { other=&p2; } ;
	virtual bool validate();
};
class textarea: public text {
public:
	int rows,cols;
	textarea(string id,string msg="") : text(id,msg) { rows=cols=-1; };
	virtual string render_input(int how);
};

class regex_field : public text {
	boost::regex const &exp;
public:
	regex_field(boost::regex const &e,string id,string msg="") : text(id,msg),exp(e)
	{ 
		low=3;
	};
	virtual bool validate();
};

class email : public regex_field {
	static boost::regex exp_email;
public:
	email(string id,string msg="") : regex_field(exp_email,id,msg) {}
};

class checkbox: public base_widget {
public:
	string input_value;
	bool value;
	checkbox(string id,string msg="") : base_widget(id,msg),input_value("1")
	{
		set(false);
	};
	virtual string render_input(int how);
	virtual string render_error();
	void set(bool v) { value=v; is_set=true; };
	void set(string const &s) { input_value=s; };
	bool get() { return value; };
	virtual void load(cgicc::Cgicc const &cgi);
};

class select : public base_widget {
	string value;
public:
	struct option {
		string value;
		string option;
	};
	list<option> select_list;
	select(string name,string msg="") : base_widget(name,msg) {};
	void add(string value,string option);
	void add(int value,string option);
	void set(string value);
	void set(int value);
	string get();
	int geti();
	virtual string render_input(int how);
	virtual bool validate();
	virtual void load(cgicc::Cgicc const &cgi);
};


class hidden : public text {
public:
	hidden(string n,string msg="") : text(n,msg) { set_nonempty(); };
	virtual string render(int how);
};

} // widgets

} //cppcms

#endif // CPPCMS_FORM_H
