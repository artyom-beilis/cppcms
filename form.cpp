#define CPPCMS_SOURCE
#include "form.h"
#include <iostream>
#include <stack>
#include <boost/format.hpp>

namespace cppcms {

base_form::base_form()
{
}
base_form::~base_form()
{
}

form::form()
{
}
form::~form()
{
	for(unsigned i=0;i<elements_.size();i++) {
		if(elements_[i].second)
			delete elements_[i].first;
	}
}


// Meanwhile -- empty
struct form::data {
// TOADD
};

void form::render(std::ostream &output,unsigned int flags)
{
	for(unsigned int i=0;i<elements_.size();i++) {
		elements_[i].first->render(output,flags);
	}
}

void form::load(http::context &cont) 
{
	for(unsigned int i=0;i<elements_.size();i++) {
		elements_[i].first->load(cont);
	}
}

bool form::validate() 
{
	bool result=true;
	for(unsigned int i=0;i<elements_.size();i++) {
		result = elements_[i].first->validate() & result;
	}
	return result;
}

void form::clear()
{
	for(unsigned int i=0;i<elements_.size();i++) {
		elements_[i].first->clear();
	}
}

void form::add(widgets::base_widget &form)
{
	elements_.push_back(widget_type(&form,false));
}
void form::add(form &subform)
{
	elements_.push_back(widget_type(&subform,false));
}

void form::attach(form *subform)
{
	elements_.push_back(widget_type(subform,true));
}

void form::attach(widgets::base_widget *subform)
{
	elements_.push_back(widget_type(subform,true));
}

struct form::iterator::data {
	std::stack<std::pair<form *,size_t> > stack;
	int pos;
	data() : pos(-1) {}	
};

form::iterator::iterator() : d(new form::iterator::data)
{
}

form::iterator::~iterator()
{
}

form::iterator::iterator(form::iterator const &other)  : d(other.d)
{
}

form::iterator const &form::iterator::operator=(form::iterator const &other)
{
	d=other.d;
	return *this;
}

bool form::iterator::equal(form::iterator const &other) const
{
	return (d->stack == other.d->stack) && (d->pos == other.d->pos);
}

void form::iterator::next()
{
	for(;;) {
		if(d->stack.empty()) {
			d->pos=-1;
			return;
		}

		form *top=d->stack.top().first;
		d->pos++;

		if((size_t)d->pos >= top->elements_.size() ) {
			d->pos=d->stack.top().second;
			d->stack.pop();
			continue;
		}

		if(dynamic_cast<widgets::base_widget *>(top->elements_[d->pos].first)!=0)
			return;

		form &f=dynamic_cast<form &>(*top->elements_[d->pos].first);
		
		d->stack.push(std::make_pair(&f,d->pos));
		d->pos=-1;
	}
}

widgets::base_widget *form::iterator::get() const
{
	return static_cast<widgets::base_widget *>(d->stack.top().first->elements_[d->pos].first);
}

form::iterator form::begin()
{
	form::iterator p;
	p.d->stack.push(std::make_pair(this,-1));
	++p;
	return p;
}

form::iterator form::end()
{
	return form::iterator();
}


namespace widgets {

////////////////////////////////
// widgets::base_widget
////////////////////////////////


struct base_widget::data {};

base_widget::base_widget() : 
	is_valid_(1),
	is_set_(0),
	is_disabled_(0)
{
}

base_widget::base_widget(std::string name) :
	name_(name), 
	is_valid_(1),
	is_set_(0),
	is_disabled_(0)
{
}

base_widget::base_widget(std::string name,std::string msg) :
	name_(name),
	message_(msg),
	is_valid_(1),
	is_set_(0),
	is_disabled_(0)
{
}

base_widget::~base_widget()
{
}

bool base_widget::set()
{
	return is_set_;
}
bool base_widget::valid()
{
	return is_valid_;
}
std::string base_widget::id()
{
	return id_;
}
std::string base_widget::name()
{
	return name_;
}
std::string base_widget::message()
{
	return message_;
}
std::string base_widget::error_message()
{
	return error_message_;
}
std::string base_widget::help()
{
	return help_;
}

void base_widget::set(bool v)
{
	is_set_=v;
}
void base_widget::valid(bool v)
{
	is_valid_=v;
}
void base_widget::id(std::string v)
{
	id_=v;
}
void base_widget::name(std::string v)
{
	name_=v;
}
void base_widget::message(std::string v)
{
	message_=v;
}
void base_widget::error_message(std::string v)
{
	error_message_=v;
}
void base_widget::help(std::string v)
{
	help_=v;
}

void base_widget::attributes_string(std::string v)
{
	attr_=v;
}

bool base_widget::disabled()
{
	return is_disabled_;
}

void base_widget::disabled(bool v)
{
	is_disabled_=v;
}

std::string base_widget::attributes_string()
{
	return attr_;
}

void base_widget::render(std::ostream &output,unsigned int flags)
{
	int how = flags & (~3);
	bool no_error = flags & error_without;
	std::string err = error_message();
	if(err.empty()) err="*";

	switch(how) {
	case as_p: output<<"<p>"; break;
	case as_table: output<<"<tr><th>"; break;
	case as_ul: output<<"<li>"; break;
	case as_dl: output<<"<dt>"; break;
	default: ;
	}
	if(!id().empty() && !message().empty()) {
		output<<"<label for=\"" << id() << "\">" << util::escape(message()) <<":</label> ";
	}
	else {
		output<<"&nbsp;";
	}
	switch(how) {
	case as_table: output<<"</th><td>"; break;
	case as_dl: output<<"</dt><dd>"; break;
	default: ;
	}

	if(!valid() && !no_error) {
		output<<"<span class=\"cppcms_form_error\">"<<util::escape(err)<<"</span> ";
	}
	else {
		output<<"&nbsp;";
	}
	output<<"<span class=\"cppcms_form_input\">";
	render_input_start(output,flags);
	output<<attr_;
	render_input_end(output,flags);
	output<<"</span>";

	if(!help().empty()) {
		output<<"<span class=\"cppcms_form_help\">"<<util::escape(help())<<"</span>";
	}
		
	switch(how) {
	case as_p: output<<"</p>\n"; break;
	case as_table: output<<"</td><tr>\n"; break;
	case as_ul: output<<"</li>\n"; break;
	case as_dl: output<<"</dd>\n"; break;
	case as_space: 
		if(flags & as_xhtml) 
			output<<"<br />\n";
		else
			output<<"<br>\n";
		break;
	default: ;
	}
}

void base_widget::clear()
{
	set(false);
}

bool base_widget::validate()
{
	valid(true);
	return true;
}

////////////////////////////////
// widgets::base_text
////////////////////////////////


struct base_text::data {};


base_text::base_text() : low_(0),high_(-1),validate_charset_(true)
{
}

base_text::base_text(std::string name) : base_widget(name), low_(0),high_(-1),validate_charset_(true)

{
}

base_text::base_text(std::string name,std::string msg) : base_widget(name,msg), low_(0),high_(-1),validate_charset_(true)

{
}

base_text::~base_text()
{
}

std::string base_text::value()
{
	if(!set())
		throw cppcms_error("Value was not loaded");
	return value_;
}

void base_text::value(std::string v)
{
	set(true);
	value_=v;
}

std::string base_text::value(std::locale const &v)
{
	//return std::use_facet<locale::charset>(v).to_utf8(value_);
	return value_;
}

void base_text::value(std::string v,std::locale const &l)
{
	//value(std::use_facet<locale::charset>(l).from_utf8(v));
	value(v);
}

void base_text::non_empty()
{
	limits(1,-1);
}

void base_text::limits(int min,int max)
{
	low_=min;
	high_=max;
}

std::pair<int,int> base_text::limits()
{
	return std::make_pair(low_,high_);
}

void base_text::validate_charset(bool v)
{
	validate_charset_=v;
}

bool base_text::validate_charset()
{
	return validate_charset_;
}

void base_text::load(http::context &context)
{
	value_.clear();
	code_points_ = 0;
	set(false);
	valid(true);
	if(name().empty()) {
		return;
	}
	http::request::form_type::const_iterator p;
	p=context.request().post_or_get().find(name());
	if(p==context.request().post_or_get().end()) {
		return;	
	}
	value_=p->second;
	set(true);
	if(validate_charset_) {
		code_points_ = 0;
	/*	locale::charset const &charset=std::use_facet<locale::charset>(context.locale().get());
		if(!charset.validate(value_,code_points_))
			valid(false);*/
	}
	else {
		code_points_=value_.size();
	}
}

bool base_text::validate()
{
	if(!valid())
		return false;
	if(!set() && low_==0 && high_==-1) {
		valid(true);
		return true;
	}
	if(code_points_ < size_t(low_) || (high_ >=0 && code_points_ > size_t(high_))) {
		valid(false);
		return false;
	}
	return true;
}


//////////////////////////////////////////////
/// widgets::text
/////////////////////////////////////////////

struct text::data {};

text::text() : size_(-1),type_("text") {}
text::~text() {}

text::text(std::string n) : base_text(n) , size_(-1),type_("text") {}
text::text(std::string n,std::string m) : base_text(n,m),size_(-1),type_("text") {}

void text::type(std::string t)
{
	type_=t;
}

void text::render_input_start(std::ostream &output,unsigned flags)
{
	output<<"<input type=\""<<type_<<"\" ";

	std::string v;
	v=id();
	if(!v.empty()) output << "id=\"" << v << "\" ";
	v=name();
	if(!v.empty()) output << "name=\"" << v << "\" ";
	if(disabled()) {
		if(flags & as_xhtml)
			output << "disabled=\"disabled\" ";
		else
			output << "disabled ";
	}
	if(size_ >= 0)
		output << boost::format("size=\"%1%\" ",std::locale::classic()) % size_;
	
	std::pair<int,int> lm=limits();
	
	if(lm.second >= 0 && validate_charset()) {
		output << boost::format("maxlength=\"%1%\" ",std::locale::classic()) % lm.second;
	}
	
	if(set()) {
		output << "value=\""<<util::escape(value())<<"\"";
	}
}

void text::render_input_end(std::ostream &output,unsigned flags)
{
	if(flags & as_xhtml)
		output<<" />";
	else
		output<<" >";
}
//////////////////////////////////////////////
/// widgets::textarea
/////////////////////////////////////////////


struct textarea::data {};

textarea::textarea() : rows_(-1), cols_(-1) {}
textarea::~textarea() {}

textarea::textarea(std::string n) : base_text(n), rows_(-1), cols_(-1) {}
textarea::textarea(std::string n,std::string m) : base_text(n,m), rows_(-1), cols_(-1)  {}

int textarea::rows() { return rows_; }
int textarea::cols() { return cols_; }

void textarea::rows(int n) { rows_ = n; }
void textarea::cols(int n) { cols_ = n; }

void textarea::render_input_start(std::ostream &output,unsigned flags)
{
	output<<"<textarea ";

	std::string v;
	v=id();
	if(!v.empty()) output << "id=\"" << v << "\" ";
	v=name();
	if(!v.empty()) output << "name=\"" << v << "\" ";
	if(disabled()) {
		if(flags & as_xhtml)
			output << "disabled=\"disabled\" ";
		else
			output << "disabled ";
	}

	if(rows_ >= 0) {
		output<<boost::format("rows=\"%1%\"",std::locale::classic()) % rows_;
	}
	if(cols_ >= 0) {
		output<<boost::format("cols=\"%1%\"",std::locale::classic()) % cols_;
	}
}

void textarea::render_input_end(std::ostream &output,unsigned flags)
{
	if(set()) {
		output << ">"<<util::escape(value())<<"</textarea>";
	}
	else {
		output << "></textarea>";
	}
}


////////////////////////
/// Password widget  ///
////////////////////////
struct password::data {};

password::password() : password_to_check_(0)
{
	type("password");
}
password::password(std::string name) : text(name), password_to_check_(0) 
{
	type("password");
}

password::password(std::string name,std::string msg) : text(name,msg), password_to_check_(0) 
{
	type("password");
}
password::~password() 
{
}

void password::check_equal(password &p2)
{
	password_to_check_ = &p2;
}

bool password::validate()
{
	if(!text::validate()) {
		value("");
		return false;
	}
	if(password_to_check_) {
		if(!password_to_check_->set() || password_to_check_->value()!=value()) {
			valid(false);
			value("");
			password_to_check_->value("");
			return false;
		}
	}
	return true;

}


} // widgets 

} // cppcms
/*


////////////////////////////////////////
/////////////////////////////////


/// Old Code





void form::append(base_form *ptr)
{
	elements.push_back(ptr);	
}
bool form::validate()
{
	bool valid=true;
	for(list<base_form *>::iterator p=elements.begin(),e=elements.end();p!=e;++p)
	{
		bool val=(*p)->validate();
		valid = valid && val;
	}
	return valid;
}

void form::clear()
{
	for(list<base_form *>::iterator p=elements.begin(),e=elements.end();p!=e;++p)
	{
		(*p)->clear();
	}
}

void form::load(cgicc::Cgicc const &cgi)
{
	for(list<base_form *>::iterator p=elements.begin(),e=elements.end();p!=e;++p)
	{
		(*p)->load(cgi);
	}
}

string form::render(int how)
{
	string output;
	for(list<base_form *>::iterator p=elements.begin(),e=elements.end();p!=e;++p) {
		output+=(*p)->render(how);
	}
	return output;
}

widgetset::~widgetset(){}
string widgetset::render(int how)
{
	string out;
	for(widgets_t::iterator p=widgets.begin(),e=widgets.end();p!=e;++p) {
		out+=(*p)->render(how);
	}
	return out;
}

widgetset::widgetset()
{
	widgets.reserve(10);
}


namespace widgets {

base_widget::base_widget(string _id,string m) : name(_id), msg(m)
{	
	is_set=false;
	is_valid=true;
}

void base_widget::clear()
{
	is_set=false;
}

bool base_widget::validate()
{
	is_valid=true;
	return true;
}

string base_widget::render(int how)
{
	string out;
	string input=render_input(how);
	string error;

	if(name.empty()) {
		throw cppcms_error("widgets::base_widget undefine name");
	}
	switch(how & error_mask) {
	case error_with:
		error=render_error();
		break;
	case error_no:
		break;
	}

	string tmp_msg;
	if(!msg.empty()){
		if(!id.empty()) {
			tmp_msg+="<lablel for=\"";
			tmp_msg+=id;
			tmp_msg+="\">";
			tmp_msg+=escape(msg);
			tmp_msg+=":</label>";
		}
		else {
			tmp_msg=escape(msg);
			tmp_msg+=":";
		}
	}

	string help_text;

	if(!help.empty()) {
		if((how & as_mask)==as_table)
			help_text=
				((how & as_xhtml) ? "<br/>" : "<br>") 
				+ escape(help);
		else
			help_text=escape(help);
	}
		
	if(error.empty()) {
		char const *frm=NULL;
		switch(how & as_mask) {
		case as_p: 
			frm = "<p>%1% %2% %3%</p>\n";
			break;
		case as_table:
			frm = "<tr><th>%1%</th><td>%2% %3%</td></tr>\n";
			break;
		case as_ul:
			frm = "<li>%1% %2% %3%</li>\n";
			break;
		case as_dl:
			frm = "<dt>%1%</dt>\n<dd>%2% %3%</dd>\n";
			break;
		case as_space:
			frm = "%1% %2% %3%\n";
			break;
		}
		assert(frm);
		out=(boost::format(frm) % tmp_msg % input % help_text).str();
	}
	else {
		char const *frm=NULL;
		switch(how & as_mask) {
		case as_p: 
			frm = "<p>%3%</p>\n<p>%1% %2% %4%</p>\n";
			break;
		case as_table:
			frm = "<tr><th>%1%</th><td>%3% %2% %4%</td></tr>\n";
			break;
		case as_ul:
			frm = "<li>%3% %1% %2% %4%</li>\n";
			break;
		case as_dl:
			frm = "<dt>%3% %1%</dt>\n<dd>%2% %4%</dd>\n";
			break;
		case as_space:
			frm = "%3% %1% %2% %4%\n";
			break;
		}
		assert(frm);
		error="<span class=\"formerror\">" + error + "</span>";
		out=(boost::format(frm) % tmp_msg % input % error % help_text).str();
	}
	return out;
}

string base_widget::render_error()
{
	if(!is_valid) {
		if(error_msg.empty()) {
			return "*";
		}
		return escape(error_msg);
	}
	return "";
}

string text::render_input(int how)
{
	string out="<input type=\"" + type +"\" name=\"";
	out+=name;
	out+="\"";
	if(!id.empty()) {
		out+=" id=\"";
		out+=id;
		out+="\"";
	}
	if(is_set) {
		out+=" value=\"";
		out+=escape(value);
		out+="\"";
	}

	if(how & as_xhtml)
		out+=" />\n";	
	else
		out+=" >\n";	
	return out;
}

void text::load(cgicc::Cgicc const &cgi)
{
	cgicc::const_form_iterator v=cgi.getElement(name);
	if(v!=cgi.getElements().end()) {
		value=**v;
		is_set=true;
	}
}

bool text::validate()
{
	if(!is_set)
		is_valid=false;
	else
		if(value.size()<low || (high>=0 && value.size()>(unsigned)high)) {
			is_valid=false;
		}
		else
			is_valid=true;
	return is_valid;
}

string &text::str()
{
	is_set=true;
	return value;
}

void text::set(string const &s)
{
	is_set=true;
	value=s;
}

string const &text::get()
{
	return value;
}

string textarea::render_input(int how)
{
	char const *frm=NULL;
	string sid;
	if(!id.empty())
		sid=" id=\"" + id +"\"";
	if(!is_set)
		frm="<textarea%3% name=\"%1%\" %2%></textarea>\n";
	else
		frm="<textarea%4% name=\"%1%\" %2%>%3%</textarea>\n";
	string format;
	if(rows!=-1 && cols!=-1)
		format=(boost::format("rows=\"%d\" cols=\"%d\"") % rows % cols).str();
	if(is_set)
		return (boost::format(frm) % name % format % escape(value) % sid).str();
	return (boost::format(frm) % name % format % sid).str();
}

string checkbox::render_input(int how)
{
	string out="<input type=\"checkbox\" name=\"";
	out+=name;
	out+="\" value=\"";
	out+=escape(input_value);
	out+="\"";
	if(is_set && value)
		out+=" checked=\"checked\"";
	if(!id.empty())
		out+=" id=\""+id+"\"";
	if(how & as_xhtml)
		out+=" />";
	else
		out+=" >";
	return out;
}

void checkbox::load(cgicc::Cgicc const &cgi)
{
	cgicc::const_form_iterator p=cgi.getElements().begin(),e=cgi.getElements().end();
	while(p!=e) {
		if(p->getName()==name && p->getValue()==input_value) {
			value=true;
			return;
		}
		++p;
	}
	value=false;
}

bool regex_field::validate()
{
	if(!text::validate())
		return false;
	if(!exp)
		throw cppcms_error("widgets::regex_field regular expression is not defined");
	is_valid=exp->match(value);
	return is_valid;
}
	
namespace {
	util::regex const exp_email("^[^@]+@[^@]+$");
}

email::email(std::string name,std::string value) : regex_field(exp_email,name,value)
{
}

void select_multiple::add(int v,string o,bool selected)
{
	add((boost::format("%d") % v).str() ,o,selected);
}

set<int> select_multiple::geti()
{
	set<int> c;
	for(set<string>::iterator p=chosen.begin(),e=chosen.end();p!=e;++p) {
		c.insert(atoi(p->c_str()));
	}
	return c;
}

bool select_multiple::validate()
{
	int count=0;
	if(!is_set) { is_valid=false; return false; };
	is_valid=true;
	for(set<string>::iterator p=chosen.begin(),e=chosen.end();p!=e;++p) {
		if(available.find(*p)==available.end()) {
			is_valid=false;
			break;
		}
		count++;
	}
	if(min>0 && count < min)
		is_valid=false;
	return is_valid;
}


void select_multiple::add(string v,string o,bool selected)
{
	available[v]=o;
	if(selected) chosen.insert(v);
}

void select_multiple::load(cgicc::Cgicc const &cgi)
{
	chosen.clear();
	cgicc::const_form_iterator p=cgi.getElements().begin(),e=cgi.getElements().end();
	for(;p!=e;++p) {
		if(p->getName()==name) {
			map<string,string>::iterator orig=available.find(p->getValue());
			if(orig!=available.end()) 
				chosen.insert(orig->first);
		}
	}
	is_set=true;
}

string select_multiple::render_input(int how)
{
	string out="<select multiple=\"multiple\" name=\"";
	out+=name;
	out+="\" ";
	if(!id.empty())
		out+="id=\""+id+"\" ";
	out+=(boost::format(" size=\"%d\"") % size).str();
	out+=">\n";
	for(map<string,string>::iterator p=available.begin(),e=available.end();p!=e;++p) {
		out+="<option value=\""+escape(p->first)+"\" ";
		if(chosen.find(p->first)!=chosen.end())
			out+=" selected=\"selected\" ";
		out+=">";
		out+=escape(p->second);
		out+="</option>\n";
	}
	out+="</select>\n";
	return out;
}

void select_multiple::clear()
{
	base_widget::clear();
	chosen.clear();
}

void select_base::add(string v,string opt)
{
	option o;
	o.value=v;
	o.option=opt;
	select_list.push_back(o);
}

void select_base::add(int val,string opt)
{
	add((boost::format("%d") % val).str(),opt);
}

void select_base::set(string v)
{
	value=v;
}

void select_base::set(int v)
{
	value=(boost::format("%d") % v).str();
}

string select_base::get()
{
	return value;
}

int select_base::geti()
{
	return atoi(value.c_str());
}

string select::render_input(int how)
{
	string out="<select name=\"";
	out+=name;
	out+="\" ";
	if(size!=-1)
		out+=(boost::format(" size=\"%d\" ") % size).str();
	if(!id.empty())
		out+="id=\""+id+"\" ";
	out+=">\n";
	for(list<option>::iterator p=select_list.begin(),e=select_list.end();p!=e;++p) {
		out+="<option value=\""+escape(p->value)+"\" ";
		if(is_set && p->value==value)
			out+=" selected=\"selected\" ";
		out+=">";
		out+=escape(p->option);
		out+="</option>\n";
	}
	out+="</select>\n";
	return out;
}

void select_base::load(cgicc::Cgicc const &cgi)
{
	cgicc::const_form_iterator p=cgi.getElement(name);
	if(p==cgi.getElements().end()) {
		is_set=false;
	}
	else {
		value=**p;
		is_set=true;
	}
}

bool select_base::validate()
{
	if(!is_set) {
		is_valid=false;
		return false;
	}
	for(list<option>::iterator p=select_list.begin(),e=select_list.end();p!=e;++p) {
		if(value==p->value) {
			is_valid=true;
			return true;
		}
	}
	is_valid=false;
	return false;
}

string radio::render_input(int how)
{
	string out="<div class=\"radio\" ";
	if(!id.empty())
		out+="id=\""+id+"\" ";
	out+=">\n";
	for(list<option>::iterator p=select_list.begin(),e=select_list.end();p!=e;++p) {
		out+="<input type=\"radio\" value=\""+escape(p->value)+"\" ";
		if(is_set && p->value==value)
			out+=" checked=\"checked\" ";
		out+="name=\"";
		out+=name;
		out+="\" ";
		if(how & as_xhtml)
			out+="/";
		out+=">";
		out+=escape(p->option);
		if(add_br) {
			if(how & as_xhtml)
				out+="<br/>";
			else
				out+="<br>";
		}
		out+="\n";
	}
	out+="</div>\n";
	return out;
}

string hidden::render(int how)
{
	string out="<input type=\"hidden\" name=\"";
	out+=name;
	out+="\" value=\"";
	out+=escape(value);
	out+="\" ";
	if(!id.empty()) {
		out+="id=\""+id+"\"";
	}
	if(how & as_xhtml)
		out+=" />\n";
	else
		out+=" >\n";

	return out;
}

bool password::validate()
{
	if(!text::validate())
		return false;
	if(other)
		if(other->value!=value) {
			is_valid=false;
			return false;
		}
	return true;
}

string password::render_input(int how)
{
	return (boost::format("<input type=\"password\" %1% name=\"%2%\" %3%>")
		% ( id.empty() ? string("") : "id=\""+id+"\"" )
		% name 
		% ( (how & as_xhtml) ? "/" : "" )
		).str();
}

string submit::render_input(int how)
{
	return (boost::format("<input type=\"submit\" %1% name=\"%2%\" value=\"%3%\" %4%>")
		% ( id.empty() ? string("") : "id=\""+id+"\"" )
		% name 
		% escape(value)
		% ( (how & as_xhtml) ? "/" : "" )
		).str();
}

void submit::load(cgicc::Cgicc const &cgi)
{
	is_set=true;
	cgicc::const_form_iterator v=cgi.getElement(name);
	if(v!=cgi.getElements().end())
		pressed=true;
}

} // Namespace widgets 

} // namespace cppcms*/
