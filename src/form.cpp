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
#define CPPCMS_SOURCE
#include <cppcms/form.h>
#include <cppcms/config.h>
#include <cppcms/encoding.h>
#include <cppcms/filters.h>
#include <cppcms/cppcms_error.h>
#include <cppcms/http_file.h>
#include <cppcms/session_interface.h>
#include <stack>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/format.hpp>
#else // Internal Boost
#   include <cppcms_boost/format.hpp>
    namespace boost = cppcms_boost;
#endif

#include <booster/regex.h>

namespace cppcms {

struct form_context::_data {};

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
struct form::_data {
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

struct form::iterator::_data {};

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
			// Elements can be only base_widget or form... so it should be safe
			current_ = static_cast<form *>(current_->elements_[offset_].first);
			return_positions_.push(offset_+1);
			offset_=0;
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


struct base_widget::_data {};

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
	message_=locale::message("NOTRANS",v);
}
void base_widget::error_message(std::string v)
{
	has_error_ = 1;
	error_message_=locale::message("NOTRANS",v);
}
void base_widget::help(std::string v)
{
	has_help_ = 1;
	help_=locale::message("NOTRANS",v);
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

void base_widget::generate(int position,form_context * /*context*/)
{
	if(is_generation_done_)
		return;
	if(name_.empty()) {
		name_ = (boost::format("_%1%",std::locale::classic()) % position).str();
	}
	is_generation_done_ = 1;
}

void base_widget::pre_load(http::context &context)
{
	context.session().validate_request_origin();
	auto_generate();
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
	html_list_type type = context.html_list();
	switch(type) {
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
			output<<"<label for=\"" << id() << "\">" << filters::escape(message()) <<"</label>";
		if(type!=as_table && type!=as_dl)
			output << "&nbsp;";
	}
	else if(type == as_table) {
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
	else if(type == as_table){
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
	case as_space: output<<"\n";
	default: ;
	}
}

void base_widget::render_attributes(form_context &context)
{
	auto_generate(&context);
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


struct base_text::_data {};


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
	pre_load(context);
	value_.clear();
	code_points_ = 0;
	set(true); // we don't care if the value was set
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

struct text::_data {};

text::~text() {}
text::text(): base_html_input("text"), size_(-1) {}
text::text(std::string const &type): base_html_input(type), size_(-1) {}

void text::size(int n) 
{
	size_ = n;
}

int text::size()
{
	return size_;
}

void text::render_value(form_context &context)
{
	if(set()) {
		context.out() << " value=\"" << util::escape(value()) << "\"";
	}
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


struct base_html_input::_data{};

base_html_input::base_html_input(std::string const &type) : type_(type) {}
base_html_input::~base_html_input() {}

void base_html_input::render_input(form_context &context)
{
	auto_generate(&context);

	std::ostream &output=context.out();
	if(context.widget_part() == first_part) {
		output<<"<input type=\""<<type_<<"\" ";
		render_attributes(context);
		render_value(context);
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


struct textarea::_data {};

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
struct password::_data {};

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
		if(!password_to_check_->set() || !set() || password_to_check_->value()!=value()) {
			valid(false);
			value("");
			password_to_check_->value("");
			return false;
		}
	}
	return true;

}


struct regex_field::_data {};
regex_field::regex_field() {}
regex_field::regex_field(booster::regex const &e) : expression_(e) {}
regex_field::regex_field(std::string const &e) : expression_(e) {}
regex_field::~regex_field() {}

void regex_field::regex(booster::regex const &e) 
{
	expression_ = e;
}

bool regex_field::validate()
{
	if(!text::validate())
		return false;
	valid(set() && booster::regex_match(value(),expression_));
	return valid();
}

struct email::_data {};
email::email() : regex_field("^[^@]+@[^@]+$") {}
email::~email() {}


struct checkbox::_data {};

checkbox::checkbox(std::string const &type) : 
	base_html_input(type),
	identification_("y"),
	value_(false)
{
	set(true);	
}
checkbox::checkbox() : 
	base_html_input("checkbox"),
	identification_("y"),
	value_(false)
{
	set(true);	
}
checkbox::~checkbox()
{
}


bool checkbox::value()
{
	return value_;
}
void checkbox::value(bool v)
{
	value_=v;
}

void checkbox::identification(std::string const &id)
{
	identification_=id;
}

std::string checkbox::identification()
{
	return identification_;
}

void checkbox::render_value(form_context &context)
{
	if(value()) {
		if(context.html() == as_xhtml)
			context.out() << " checked=\"checked\" ";
		else
			context.out() << " checked ";
	}
	context.out() << "value=\""<<util::escape(identification_)<<"\" ";
}

void checkbox::load(http::context &context)
{
	pre_load(context);
	set(true);
	std::pair<http::request::form_type::const_iterator,http::request::form_type::const_iterator> 
		range=context.request().post_or_get().equal_range(name());
	value(false);
	while(range.first != range.second) {
		if(range.first->second == identification_) {
			value(true);
			break;
		}
		++range.first;
	}
}

struct select_multiple::_data{};


select_multiple::element::element():
	selected(0),
	need_translation(0),
	original_select(0)
{
}

select_multiple::element::element(std::string const &val,locale::message const &msg,bool sel) :
	selected(sel),
	need_translation(1),
	original_select(sel),
	id(val),
	tr_option(msg)
{
}

select_multiple::element::element(std::string const &val,std::string const &msg,bool sel) :
	selected(sel),
	need_translation(0),
	original_select(sel),
	id(val),
	str_option(msg)
{
}


select_multiple::select_multiple() :
	low_(0),
	high_(std::numeric_limits<unsigned>::max()),
	rows_(0)
{
}

select_multiple::~select_multiple()
{
}
void select_multiple::add(std::string const &opt,std::string const &id,bool selected)
{
	elements_.push_back(element(id,opt,selected));
}

void select_multiple::add(locale::message const &opt,std::string const &id,bool selected)
{
	elements_.push_back(element(id,opt,selected));
}

void select_multiple::add(std::string const &opt,bool selected)
{
	std::string id=(boost::format("%1%",std::locale::classic()) % elements_.size()).str();
	elements_.push_back(element(id,opt,selected));
}

void select_multiple::add(locale::message const &opt,bool selected)
{
	std::string id=(boost::format("%1%",std::locale::classic()) % elements_.size()).str();
	elements_.push_back(element(id,opt,selected));
}

std::vector<bool> select_multiple::selected_map()
{
	std::vector<bool> flags(elements_.size(),false);
	for(unsigned i=0;i<elements_.size();i++)
		flags[i]=elements_[i].selected;
	return flags;
}

std::set<std::string> select_multiple::selected_ids()
{
	std::set<std::string> ids;
	for(unsigned i=0;i<elements_.size();i++)
		if(elements_[i].selected)
			ids.insert(elements_[i].id);
	return ids;
}

void select_multiple::clear()
{
	for(unsigned i=0;i<elements_.size();i++){
		elements_[i].selected=elements_[i].original_select;
	}
}

void select_multiple::load(http::context &context)
{
	pre_load(context);
	set(true);
	std::pair<http::request::form_type::const_iterator,http::request::form_type::const_iterator> 
		range=context.request().post_or_get().equal_range(name());
	std::set<std::string> keys;
	for(http::request::form_type::const_iterator p=range.first;p!=range.second;++p)
		keys.insert(p->second);
	for(unsigned i=0;i<elements_.size();i++) {
		elements_[i].selected= keys.find(elements_[i].id) != keys.end();
	}
}

bool select_multiple::validate()
{
	unsigned count=0;
	for(unsigned i=0;i<elements_.size();i++)
		count += elements_[i].selected;
	if(low_ <= count && count <= high_) {
		valid(true);
		return true;
	}
	else {
		valid(false);
		return false;
	}
}

void select_multiple::render_input(form_context &context)
{
	auto_generate(&context);
	std::ostream &out=context.out();
	if(context.widget_part() == first_part) {
		if(context.html() == as_xhtml)
			out<<"<select multiple=\"multiple\" ";
		else
			out<<"<select multiple ";
		if(rows_ > 0)
			out << boost::format(" size=\"%1%\" ",std::locale::classic()) % rows_;
		render_attributes(context);
	}
	else {
		out<<" >\n";
		for(unsigned i=0;i<elements_.size();i++) {
			element &el=elements_[i];
			out << "<option value=\"" << util::escape(el.id) <<"\" ";
			if(el.selected) {
				if(context.html() == as_xhtml)
					out << "selected=\"selected\" ";
				else
					out << "selected ";
			}
			out << ">";
			if(el.need_translation)
				out << filters::escape(el.tr_option);
			else
				out << util::escape(el.str_option);
			out << "</option>\n";
		}
		out << "</select>";
	}
}

void select_multiple::non_empty()
{
	at_least(1);
}


void select_multiple::at_least(unsigned l)
{
	low_ = l;
}

void select_multiple::at_most(unsigned h)
{
	high_ = h;
}

unsigned select_multiple::at_least()
{
	return low_;
}

unsigned select_multiple::at_most()
{
	return high_;
}

unsigned select_multiple::rows()
{
	return rows_;
}

void select_multiple::rows(unsigned v)
{
	rows_=v;
}

////////////////////
// Select base
///////////////////

struct select_base::_data {};
struct select_base::element::_data {};
select_base::element::element() : need_translation(0)
{
}

select_base::element::element(std::string const &v,std::string const &msg) :
	need_translation(0),
	id(v),
	str_option(msg)
{
}

select_base::element::element(std::string const &v,locale::message const &msg) :
	need_translation(1),
	id(v),
	tr_option(msg)
{
}

select_base::element::element(select_base::element const &other) : 
	need_translation(other.need_translation),
	reserved(other.reserved),
	id(other.id),
	str_option(other.str_option),
	tr_option(other.tr_option)
{
}

select_base::element const &select_base::element::operator=(select_base::element const &other)
{
	if(this!=&other) {
		need_translation = other.need_translation;
		reserved = other.reserved;
		id=other.id;
		str_option=other.str_option;
		tr_option=other.tr_option;
	}
	return *this;
}

select_base::element::~element()
{
}


select_base::select_base() : selected_(-1), default_selected_(-1), non_empty_(0)
{
}

select_base::~select_base() 
{
}

void select_base::add(std::string const &str,std::string const &id)
{
	elements_.push_back(element(id,str));
}

void select_base::add(locale::message const &str,std::string const &id)
{
	elements_.push_back(element(id,str));
}

void select_base::add(std::string const &str)
{
	std::string id=(boost::format("%1%",std::locale::classic()) % elements_.size()).str();
	elements_.push_back(element(id,str));
}

void select_base::add(locale::message const &str)
{
	std::string id=(boost::format("%1%",std::locale::classic()) % elements_.size()).str();
	elements_.push_back(element(id,str));
}

void select_base::selected(int no)
{
	if(no >= int(elements_.size()))
		throw cppcms_error("select_base::invalid index");
	else if(no < 0) no = -1;
	selected_ = no;
	default_selected_ = no;
}
void select_base::selected_id(std::string id)
{
	if(id.empty()) {
		selected_ = -1;
		default_selected_ = -1;
		return;
	}
	for(unsigned i=0;i<elements_.size();i++) {
		if(id==elements_[i].id) {
			selected_ = i;
			default_selected_ = i;
			return;
		}
	}
	throw cppcms_error("Select base::invalid index: " + id);
}

void select_base::clear()
{
	selected_ = default_selected_;
}

void select_base::non_empty()
{
	non_empty_ = 1;
}

int select_base::selected()
{ 
	return selected_; 
}

std::string select_base::selected_id()
{
	if(selected_ < 0 || selected_ >= int(elements_.size()))
		return "";
	return elements_[selected_].id;
}

bool select_base::validate()
{
	if(non_empty_ && selected_ == -1)
		valid(false);
	else
		valid(true);
	return valid();
}

void select_base::load(http::context &context)
{
	pre_load(context);
	set(true);
	std::pair<http::request::form_type::const_iterator,http::request::form_type::const_iterator> 
		range=context.request().post_or_get().equal_range(name());
	selected_ = -1;

	http::request::form_type::const_iterator p=range.first;
	// If empty or more then one
	if(range.first == range.second || (++range.first)!=range.second )
		return;
	std::string key = p->second;
	for(unsigned i=0;i<elements_.size();i++) {
		if(elements_[i].id == key) {
			selected_ = i;
			break;
		}
	}
}




////////////////
/// select
///////////////////

struct select::_data {};

select::select() {}
select::~select() {}

void select::render_input(form_context &context)
{
	auto_generate(&context);
	std::ostream &out=context.out();
	if(context.widget_part() == first_part) {
		out<<"<select ";
		render_attributes(context);
	}
	else {
		out<<" >\n";
		for(unsigned i=0;i<elements_.size();i++) {
			element &el=elements_[i];
			out << "<option value=\"" << util::escape(el.id) <<"\" ";
			if(int(i) == selected()) {
				if(context.html() == as_xhtml)
					out << "selected=\"selected\" ";
				else
					out << "selected ";
			}
			out << ">";
			if(el.need_translation)
				out << filters::escape(el.tr_option);
			else
				out << util::escape(el.str_option);
			out << "</option>\n";
		}
		out << "</select>";
	}
}
///////////////////////////
/// Radio
/////////////////////////

struct radio::_data {};

radio::radio() : vertical_(1)
{
}
radio::~radio() {}

void radio::vertical(bool v) 
{
	vertical_ = v;
}

bool radio::vertical()
{
	return vertical_;
}

void radio::render_input(form_context &context)
{
	auto_generate(&context);
	std::ostream &out=context.out();
	if(context.widget_part() == first_part) {
		out<<"<div class=\"cppcms_radio\" ";
		if(!id().empty()) out << "id=\"" << id() << "\" ";
	}
	else {
		out<<" >\n";
		for(unsigned i=0;i<elements_.size();i++) {
			element &el=elements_[i];
			out 	<< "<input type=\"radio\" value=\"" 
				<< util::escape(el.id) <<"\" ";
			if(!name().empty()) {
				out<< "name=\"" << name() << "\" ";
			}
			
			if(int(i) == selected()) {
				if(context.html() == as_xhtml)
					out << "checked=\"checked\" ";
				else
					out << "checked ";
			}

			if(disabled()) {
				if(context.html() == as_xhtml)
					out << "disabled=\"disabled\" ";
				else
					out << "disabled ";
			}

			if(context.html() == as_xhtml)
				out << "/> ";
			else
				out << "> ";
			if(el.need_translation)
				out << filters::escape(el.tr_option);
			else
				out << util::escape(el.str_option);
			if(vertical_)
				if(context.html() == as_xhtml)
					out << "<br/>\n";
				else
					out << "<br>\n";
			else
				out << "\n";
		}
		out << "</div>";
	}
}


//////////////
// Submit
////////////////

struct submit::_data {};
submit::submit() : 
	base_html_input("submit"),
	pressed_(false)
{
	set(true);
}
submit::~submit() 
{
}

void submit::value(std::string val)
{
	value_=locale::message("NOTRANS",val);
}
void submit::value(locale::message const &msg)
{
	value_=msg;
}

void submit::render_value(form_context &context)
{
	context.out() << "value=\"" << filters::escape(value_) << "\" ";
}

void submit::load(http::context &context)
{
	pre_load(context);
	set(true);
	pressed_ = context.request().post_or_get().find(name()) != context.request().post_or_get().end();
}

bool submit::value()
{
	return pressed_;
}

struct hidden::_data{};
hidden::hidden() : text("hidden") {}
hidden::~hidden() {}

void hidden::render(form_context &context)
{
	auto_generate(&context);
	std::ostream &output = context.out();
	
	context.widget_part(first_part);
	render_input(context);

	output<<attributes_string();

	context.widget_part(second_part);
	render_input(context);

}


struct file::_data{};

file::file() : 
	base_html_input("file"),
	size_min_(-1),
	size_max_(-1),
	check_charset_(1),
	check_non_empty_(0)
{
}

file::~file()
{
}

void file::non_empty() 
{
	check_non_empty_=1;
}

void file::limits(int min,int max)
{
	size_min_ = min;
	size_max_ = max;
}

std::pair<int,int> file::limits()
{
	return std::pair<int,int>(size_min_,size_max_);
}

void file::validate_filename_charset(bool v)
{
	check_charset_=v ? 1 : 0;
}

bool file::validate_filename_charset()
{
	return check_charset_;
}

booster::shared_ptr<http::file> file::value()
{
	if(!set())
		throw cppcms_error("File was not loaded");
	return file_;
}

void file::mime(std::string const &s)
{
	mime_string_=s;
	mime_regex_ = booster::regex();
}

void file::mime(booster::regex const &r)
{
	mime_string_.clear();
	mime_regex_ = r;
}

void file::add_valid_magic(std::string const &m)
{
	magics_.push_back(m);
}

void file::load(http::context &context)
{
	pre_load(context);
	set(false);
	valid(true);
	if(name().empty())
		return;
	std::string const field_name = name();
	std::vector<booster::shared_ptr<http::file> > files = context.request().files();
	for(unsigned i=0;i<files.size();i++) {
		if(files[i]->name()==field_name) {
			file_=files[i];
			set(true);
			break;
		}
	}
	if(set()) {
		std::string file_name = file_->filename();
		if(check_charset_) {
			size_t count=0;
			if(!encoding::valid(context.locale(),file_name.c_str(),file_name.c_str()+file_name.size(),count)) {
				valid(false);
			}
		}
	}
}
void file::render_value(form_context &/*context*/)
{
	// Nothing really to do
}
bool file::validate()
{
	if(!set()) {
		if(check_non_empty_) {
			valid(false);
			return false;
		}
		else {
			valid(true);
			return true;
		}
	}
	if(!valid())
		return false;
	if(size_max_ != -1 || size_min_!= -1) {
		size_t file_size = file_->size();
		if(size_max_ != -1 && file_size > size_t(size_max_)) {
			valid(false);
			return false;
		}
		if(size_min_ != -1 && file_size < size_t(size_min_)) {
			valid(false);
			return false;
		}
	}
	if(!mime_string_.empty() && file_->mime()!=mime_string_) {
		valid(false);
		return false;
	}
	if(!mime_regex_.empty() && !booster::regex_match(file_->mime(),mime_regex_)){
		valid(false);
		return false;
	}
	if(!filename_regex_.empty() && !booster::regex_match(file_->filename(),filename_regex_)){
		valid(false);
		return false;
	}
	if(valid() && !magics_.empty()) {
		size_t size_max = 0;
		for(unsigned i=0;i<magics_.size();i++)
			size_max=std::max(magics_[i].size(),size_max);
		std::vector<char> buf(size_max+1,0);
		file_->data().seekg(0);
		file_->data().read(&buf.front(),size_max);
		std::string magic(&buf.front(),file_->data().gcount());
		file_->data().seekg(0);
		valid(false);
		for(unsigned i=0;i<magics_.size();i++) {
			if(magic.compare(0,magics_[i].size(),magics_[i])==0) {
				valid(true);
				break;
			}
		}
	}
	return valid();

}

booster::regex file::filename()
{
	return filename_regex_;
}

void file::filename(booster::regex const &fn)
{
	filename_regex_ = fn;
}


} // widgets 




} // cppcms


