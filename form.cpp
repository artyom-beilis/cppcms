#define CPPCMS_SOURCE
#include "form.h"
#include "config.h"
#include "encoding.h"
#include "regex.h"
#include "filters.h"
#include <iostream>
#include <stack>
#include <boost/format.hpp>

namespace cppcms {

struct form_context::data {};

form_context::form_context() :
	html_type_(as_html),
	html_list_type_(as_p),
	widget_part_type_(first_part),
	output_(0)
{
}

form_context::form_context(std::ostream &out,form_context::html_type t,form_context::html_list_type hlt) :
	html_type_(t),
	html_list_type_(hlt),
	widget_part_type_(first_part),
	output_(&out)
{
}

void form_context::html(html_type t)
{
	html_type_=t;
}

void form_context::html_list(html_list_type t)
{
	html_list_type_ = t;
}

void form_context::widget_part(widget_part_type t)
{
	widget_part_type_ = t;
}

void form_context::out(std::ostream &out)
{
	output_ = &out;
}
std::ostream &form_context::out() const
{
	if(!output_)
		throw cppcms_error("Can't use form context without assigned output");
	return *output_;
}

form_flags::html_type form_context::html() const
{
	return static_cast<html_type>(html_type_);
}

form_flags::html_list_type form_context::html_list() const
{
	return static_cast<html_list_type>(html_list_type_);
}

form_flags::widget_part_type form_context::widget_part() const
{
	return static_cast<widget_part_type>(widget_part_type_);
}


form_context::~form_context()
{
}

base_form::base_form()
{
}
base_form::~base_form()
{
}

// Meanwhile -- empty
struct form::data {
// TOADD
};

form::form() : parent_(0)
{
}
form::~form()
{
	for(unsigned i=0;i<elements_.size();i++) {
		if(elements_[i].second)
			delete elements_[i].first;
	}
}

void form::parent(base_form *parent)
{
	parent_=&dynamic_cast<form &>(*parent);
}

form *form::parent()
{
	return parent_;
}


void form::render(form_context &context)
{
	for(unsigned int i=0;i<elements_.size();i++) {
		elements_[i].first->render(context);
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
	form.parent(this);
}
void form::add(form &subform)
{
	elements_.push_back(widget_type(&subform,false));
	subform.parent(this);
}

void form::attach(form *subform)
{
	elements_.push_back(widget_type(subform,true));
	subform->parent(this);
}

void form::attach(widgets::base_widget *subform)
{
	elements_.push_back(widget_type(subform,true));
	subform->parent(this);
}

struct form::iterator::data {};

form::iterator::iterator() : current_(0), offset_(0)
{
}

form::iterator::iterator(form &f) : current_(&f), offset_(0)
{
	next();
}

form::iterator::~iterator()
{
}

form::iterator::iterator(form::iterator const &other) : 
	return_positions_(other.return_positions_),
	current_(other.current_),
	offset_(other.offset_),
	d(other.d)
{
}

form::iterator const &form::iterator::operator=(form::iterator const &other)
{
	if(this != &other) {
		return_positions_ = other.return_positions_;
		current_ = other.current_;
		offset_=other.offset_;
		d=other.d;
	}
	return *this;
}

bool form::iterator::equal(form::iterator const &other) const
{
	return 	current_ == other.current_ 
		&& offset_ == other.offset_
		&& return_positions_ == other.return_positions_;
}

void form::iterator::zero()
{
	current_ = 0;
	offset_ = 0;
}

void form::iterator::next()
{
	for(;;) {
		if(!current_)
			return;
		if(offset_ >=current_->elements_.size()) {
			if(return_positions_.empty()) {
				zero();
				return;
			}
			offset_ = return_positions_.top();
			return_positions_.pop();
			current_ = current_->parent();
		}
		else if(dynamic_cast<widgets::base_widget *>(current_->elements_[offset_].first)!=0) {
			offset_++;
			return;
		}
		else {
			return_positions_.push(offset_+1);
			offset_=0;
			// Elements can be only base_widget or form... so it should be safe
			current_ = static_cast<form *>(current_->elements_[offset_].first);
		}
	}
}

widgets::base_widget *form::iterator::get() const
{
	return static_cast<widgets::base_widget *>(current_->elements_[offset_ - 1].first);
}

form::iterator form::begin()
{
	return form::iterator(*this);
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
	parent_(0),
	is_valid_(1),
	is_set_(0),
	is_disabled_(0),
	is_generation_done_(0),
	has_message_(0),
	has_error_(0),
	has_help_(0)
{
}

base_widget::~base_widget()
{
}

bool base_widget::has_message()
{
	return has_message_;
}

bool base_widget::has_help()
{
	return has_help_;
}

bool base_widget::has_error_message()
{
	return has_error_;
}


void base_widget::parent(base_form *parent)
{
	parent_=&dynamic_cast<form &>(*parent);
}

form *base_widget::parent()
{
	return parent_;
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
locale::message base_widget::message()
{
	return message_;
}
locale::message base_widget::error_message()
{
	return error_message_;
}
locale::message base_widget::help()
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

void base_widget::message(locale::message const &v)
{
	has_message_ = 1;
	message_=v;
}
void base_widget::error_message(locale::message const &v)
{
	has_error_ = 1;
	error_message_=v;
}
void base_widget::help(locale::message const &v)
{
	has_help_ = 1;
	help_=v;
}

void base_widget::message(std::string v)
{
	has_message_ = 1;
	message_=locale::message("#NOTRANS#" + v);
}
void base_widget::error_message(std::string v)
{
	has_error_ = 1;
	error_message_=locale::message("#NOTRANS#" + v);
}
void base_widget::help(std::string v)
{
	has_help_ = 1;
	help_=locale::message("#NOTRANS#" + v);
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

void base_widget::generate(int position,form_context *context)
{
	if(is_generation_done_)
		return;
	if(name_.empty()) {
	#ifdef HAVE_SNPRINTF
		char buf[32];
		snprintf(buf,sizeof(buf),"_%d",position);
		name_=buf;
	#else
		name_ = (boost::format("_%1$d",std::locale::classic()) % position).str();
	#endif
	}
	is_generation_done_ = 1;
}

void base_widget::auto_generate(form_context *context)
{
	if(is_generation_done_)
		return;
	if(parent() == 0) {
		generate(1,context);
		return;
	}
	form *top;
	for(top = parent();top->parent();top=top->parent())
		;
	int i=1;
	form::iterator p=top->begin(),e=top->end();
	while(p!=e) {
		p->generate(i,context);
		++p;
		++i;
	}
}

void base_widget::render(form_context &context)
{
	auto_generate(&context);
	std::ostream &output = context.out();
	switch(context.html_list()) {
	case as_p: output<<"<p>"; break;
	case as_table: output<<"<tr><th>"; break;
	case as_ul: output<<"<li>"; break;
	case as_dl: output<<"<dt>"; break;
	default: ;
	}
	
	if(has_message()) {
		if(id_.empty())
			output << filters::escape(message());
		else
			output<<"<label for=\"" << id() << "\">" << filters::escape(message()) <<":</label> ";
	}
	else {
		output<<"&nbsp;";
	}
	switch(context.html_list()) {
	case as_table: output<<"</th><td>"; break;
	case as_dl: output<<"</dt><dd>"; break;
	default: ;
	}

	if(!valid()) {
		output<<"<span class=\"cppcms_form_error\">";
		if(has_error_message())
			output<<filters::escape(error_message());
		else
			output<<"*";
		output<<"</span> ";
	}
	else {
		output<<"&nbsp;";
	}
	output<<"<span class=\"cppcms_form_input\">";
	context.widget_part(first_part);
	render_input(context);
	output<<attr_;
	context.widget_part(second_part);
	render_input(context);
	output<<"</span>";

	if(has_help()) {
		output<<"<span class=\"cppcms_form_help\">"<<filters::escape(help())<<"</span>";
	}
		
	switch(context.html_list()) {
	case as_p: output<<"</p>\n"; break;
	case as_table: output<<"</td><tr>\n"; break;
	case as_ul: output<<"</li>\n"; break;
	case as_dl: output<<"</dd>\n"; break;
	case as_space: 
		if(context.html() == as_xhtml) 
			output<<"<br />\n";
		else
			output<<"<br>\n";
		break;
	default: ;
	}
}

void base_widget::render_attributes(form_context &context)
{
	std::ostream &output = context.out();
	if(!id_.empty()) output << "id=\"" << id_ << "\" ";
	if(!name_.empty()) output << "name=\"" << name_ << "\" ";
	if(disabled()) {
		if(context.html() == as_xhtml)
			output << "disabled=\"disabled\" ";
		else
			output << "disabled ";
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
	auto_generate();
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
		if(!encoding::valid(context.locale(),value_.data(),value_.data()+value_.size(),code_points_))
			valid(false);
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

text::~text() {}
text::text(): base_html_input("text"), size_(-1) {}
text::text(std::string const &type): base_html_input(type), size_(-1) {}
void text::render_value(form_context &context)
{
	context.out() << " value=\"" << util::escape(value()) << "\"";
}

void text::render_attributes(form_context &context)
{
	base_widget::render_attributes(context);

	std::ostream &output = context.out();
	if(size_ >= 0)
		output << boost::format("size=\"%1%\" ",std::locale::classic()) % size_;
	std::pair<int,int> lm=limits();
	if(lm.second >= 0 && validate_charset()) {
		output << boost::format("maxlength=\"%1%\" ",std::locale::classic()) % lm.second;
	}
}

struct base_html_input::data{};

base_html_input::base_html_input(std::string const &type) : type_(type) {}
base_html_input::~base_html_input() {}

void base_html_input::render_input(form_context &context)
{
	std::ostream &output=context.out();
	if(context.widget_part() == first_part) {
		output<<"<input type=\""<<type_<<"\" ";
		render_attributes(context);
	}
	else {
		if(context.html() ==  as_xhtml)
			output<<" />";
		else
			output<<" >";
	}
}

//////////////////////////////////////////////
/// widgets::textarea
/////////////////////////////////////////////


struct textarea::data {};

textarea::textarea() : rows_(-1), cols_(-1) {}
textarea::~textarea() {}

int textarea::rows() { return rows_; }
int textarea::cols() { return cols_; }

void textarea::rows(int n) { rows_ = n; }
void textarea::cols(int n) { cols_ = n; }

void textarea::render_input(form_context &context)
{
	std::ostream &output = context.out();

	if(context.widget_part() == first_part)
	{
		output<<"<textarea ";
		render_attributes(context);

		if(rows_ >= 0) {
			output<<boost::format("rows=\"%1%\"",std::locale::classic()) % rows_;
		}

		if(cols_ >= 0) {
			output<<boost::format("cols=\"%1%\"",std::locale::classic()) % cols_;
		}
	}
	else {
		if(set()) {
			output << ">"<<util::escape(value())<<"</textarea>";
		}
		else {
			output << "></textarea>";
		}
	}
}

////////////////////////
/// Password widget  ///
////////////////////////
struct password::data {};

password::password() : text("password"), password_to_check_(0)
{
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


struct regex_field::data {};
regex_field::regex_field() : expression_(0) {}
regex_field::regex_field(util::regex const &e) : expression_(&e) {}
regex_field::~regex_field() {}

void regex_field::regex(util::regex const &e) 
{
	expression_ = &e;
}
bool regex_field::validate()
{
	if(!text::validate())
		return false;
	valid(expression_->match(value()));
	return valid();
}

struct email::data {};
namespace {
	util::regex email_regex("^[^@]+@[^@]+$");
}

email::email() : regex_field(email_regex) {}
email::~email() {}

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
