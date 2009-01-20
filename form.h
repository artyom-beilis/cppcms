#ifndef CPPCMS_FORM_H
#define CPPCMS_FORM_H 
#include <string>
#include <set>
#include <list>
#include <cgicc/Cgicc.h>
#include <boost/regex.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <ostream>

namespace cppcms {
using namespace std;
class base_form {
public:
	virtual ~base_form();
	enum {	as_html = 0,
		as_xhtml= 1,
		as_p	= 0,
		as_table= 1<<1,
		as_ul 	= 2<<1,
		as_dl	= 3<<1,
		as_space= 4<<1,
		as_mask = 7<<1,
		error_with   = 0,
		error_no = 0x10,
		error_mask = 0x10, };

	virtual string render(int how) = 0;
	virtual void load(cgicc::Cgicc const &cgi) = 0;
	inline void load(cgicc::Cgicc const *cgi) { if(cgi) load(*cgi); }
	virtual bool validate() = 0;
	virtual void clear() = 0;
};

class form : public boost::noncopyable , public base_form {
protected:
	list<base_form *> elements;
public:
	void append(base_form *ptr);
	inline form &operator & (base_form &f) { append(&f); return *this; }
	virtual string render(int how);
	virtual void load(cgicc::Cgicc const &cgi);
	virtual bool validate();
	virtual void clear();
};




namespace widgets {

class base_widget : public base_form {
public:
	bool is_set;
	base_widget(string name="",string msg="");
	string id;
	string name;
	string msg;
	string error_msg;
	string help;
	bool is_valid;
	virtual string render(int how);
	virtual string render_input(int ) = 0;
	virtual string render_error();
	virtual void clear();
	virtual bool validate();
	void not_valid() { is_valid=false; }
};

class text : public base_widget {
protected:
	string type;
	unsigned low;
	int high;
public:
	string value;
	text(string name="",string msg="") : base_widget(name,msg) , type("text"){ low=0,high=-1; };
	void set_limits(int min,int max) { low=min,high=max; }
	void set_nonempty() { low = 1, high=-1;};
	void set_limits(string e,int min,int max) { error_msg=e; set_limits(min,max); }
	void set_nonempty(string e){ error_msg=e; set_nonempty(); };
	virtual string render_input(int how);
	void set(string const &s);
	string &str();
	string const &get();
	virtual bool validate();
	virtual void load(cgicc::Cgicc const &cgi);
};

template<typename T>
class number: public text {
	T min,max,value;
	bool check_low,check_high;
public:
	number(string name="",string msg="") :
		text(name,msg),
		value(0),check_low(false),check_high(false)
	{};
	void set_low(T a)
	{
		min=a;
		check_low=true;
		set_nonempty();
	}
	void set_high(T b)
	{
		max=b;
		check_high=true;
		set_nonempty();
	}
	void set_range(T a,T b)
	{
		min=a; max=b;
		check_low=check_high=true;
		set_nonempty();
	}
	virtual bool validate() 
	{
		if(!text::validate())
			return false;
		if(!str().empty()) {
			try {
				value=boost::lexical_cast<T>(str());
			}
			catch(boost::bad_lexical_cast const &e) {
				return (is_valid=false);
			}
		}
		if((check_low || check_high) && str().empty())
			return (is_valid=false);
		if(check_low && value<min)
			return (is_valid=false);
		if(check_high && value>max)
			return (is_valid=false);
		return (is_valid=true);
	}
	T get() const { return value; };
	T &val() { return value; }
};

class password: public text {
	password *other;
public:
	password(string name="",string msg="") : text(name,msg),other(0) {} ;
	void set_equal(password &p2) { other=&p2; } ;
	virtual bool validate();
	virtual string render_input(int how);
};
class textarea: public text {
public:
	int rows,cols;
	textarea(string name="",string msg="") : text(name,msg) { rows=cols=-1; };
	virtual string render_input(int how);
};

class regex_field : public text {
	boost::regex const *exp;
public:
	regex_field() : exp(0) {}	
	regex_field(boost::regex const &e,string name="",string msg="") : text(name,msg),exp(&e) {}
	virtual bool validate();
};

class email : public regex_field {
	static boost::regex exp_email;
public:
	email(string name="",string msg="") : regex_field(exp_email,name,msg) {}
};

class checkbox: public base_widget {
public:
	string input_value;
	bool value;
	checkbox(string name="",string msg="") : base_widget(name,msg),input_value("1")
	{
		set(false);
	};
	virtual string render_input(int how);
	void set(bool v) { value=v; is_set=true; };
	void set(string const &s) { input_value=s; };
	bool get() { return value; };
	virtual void load(cgicc::Cgicc const &cgi);
};

class select_multiple : public base_widget {
	int min;
public:
	int size;
	select_multiple(string name="",int s=0,string msg="") : base_widget(name,msg),min(-1),size(s) {};
	set<string> chosen;
	map<string,string> available;
	void add(string val,string opt,bool selected=false);
	void add(int val,string opt,bool selected=false);
	void add(string v,bool s=false) { add(v,v,s); }
	void add(int v,bool s=false) { add(v,boost::lexical_cast<std::string>(v),s); }
	set<string> &get() { return chosen; };
	set<int> geti();
	void set_min(int n) { min=n; };
	virtual string render_input(int how);
	virtual bool validate();
	virtual void load(cgicc::Cgicc const &cgi);
	virtual void clear();
};

class select_base : public base_widget {
protected:
	string value;
	bool has_value;
public:
	struct option {
		string value;
		string option;
	};
	list<option> select_list;
	select_base(string name="",string msg="") : base_widget(name,msg){};
	void add(string value,string option);
	void add(string v) { add(v,v); }
	void add(int value,string option);
	void add(int v) { add(v,boost::lexical_cast<std::string>(v)); }
	void set(string value);
	void set(int value);
	string get();
	int geti();
	virtual bool validate();
	virtual void load(cgicc::Cgicc const &cgi);
};

class select : public select_base {
	int size;
public:
	select(string n="",string m="") : select_base(n,m),size(-1) {};
	void set_size(int n) { size=n;}
	virtual string render_input(int how);
};

class radio : public select_base {
	bool add_br;
public:
	radio(string name="",string msg="") : select_base(name,msg),add_br(false) {}
	void set_vertical() { add_br=true; }
	virtual string render_input(int how);
};

class hidden : public text {
public:
	hidden(string n="",string msg="") : text(n,msg) { set_nonempty(); };
	virtual string render(int how);
};

class submit : public base_widget {
public:
	string value;
	bool pressed;
	submit(string name="",string button="",string msg="") : base_widget(name,msg), value(button),pressed(false) {};
	virtual string render_input(int);
	virtual void load(cgicc::Cgicc const &cgi);
};

} // widgets

class widgetset {
public:
	typedef vector<widgets::base_widget*> widgets_t;
	widgets_t widgets;
	inline widgetset &operator<<(widgets::base_widget &w) { widgets.push_back(&w); return *this; }
	widgetset();
	virtual string render(int how);
	virtual ~widgetset();
};

} //cppcms

#endif // CPPCMS_FORM_H
