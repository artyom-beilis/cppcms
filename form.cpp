#include "form.h"
#include "util.h"

#include <boost/format.hpp>
#include <iostream>

namespace cppcms {
base_form::~base_form()
{
}

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
	is_valid=boost::regex_match(value,exp);
	return is_valid;
}

boost::regex email::exp_email("^[^@]+@[^@]+$");

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
		if(add_br)
			if(how & as_xhtml)
				out+="<br/>";
			else
				out+="<br>";
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

} // namespace cppcms
