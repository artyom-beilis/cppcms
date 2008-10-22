#include "form.h"
#include "util.h"

#include <boost/format.hpp>

namespace cppcms {
base_form::~base_form()
{
}

void base_form::append(base_form *ptr)
{
	elements.push_back(ptr);	
}
bool base_form::validate()
{
	bool valid=true;
	for(list<base_form *>::iterator p=elements.begin(),e=elements.end();p!=e;++p)
	{
		valid = valid && (*p)->validate();
	}
	return valid;
}

void base_form::load(cgicc::Cgicc const &cgi)
{
	for(list<base_form *>::iterator p=elements.begin(),e=elements.end();p!=e;++p)
	{
		(*p)->load(cgi);
	}
}

string base_form::render(int how)
{
	string output;
	for(list<base_form *>::iterator p=elements.begin(),e=elements.end();p!=e;++p) {
		output+=(*p)->render(how);
	}
	return output;
}

namespace widgets {

base_widget::base_widget(string _id,string m) : name(_id), msg(m)
{	
	is_set=false;
	is_valid=false;
}

string base_widget::render(int how)
{
	string out;
	string input=render_input(how);
	string error;

	switch(how & error_mask) {
	case error_only:
		return render_error();
	case error_with:
		error=render_error();
		break;
	case error_no:
		break;
	}
	string tmp_msg;
	if(!id.empty()) {
		tmp_msg="<lablel for=\"" + id + "\">" + msg +"</label>";
	}
	else {
		tmp_msg=name;
	}

	if(error.empty()) {
		char const *frm=NULL;
		switch(how & as_mask) {
		case as_p: 
			frm = "<p>%1%: %2%</p>\n";
			break;
		case as_table:
			frm = "<tr><th>%1%</th><td>%2%</td></tr>\n";
			break;
		case as_ul:
			frm = "<li>%1%: %2%</li>\n";
			break;
		}
		assert(frm);
		out=(boost::format(frm) % tmp_msg % input).str();
	}
	else {
		char const *frm=NULL;
		switch(how & as_mask) {
		case as_p: 
			frm = "<p>%3%</p>\n<p>%1%: %2%</p>\n";
			break;
		case as_table:
			frm = "<tr><th>%1%</th><td>%3% %2%</td></tr>\n";
			break;
		case as_ul:
			frm = "<li>%3% %1%: %2%</li>\n";
			break;
		}
		assert(frm);
		error="<ul class=\"errorlist\">" + error + "</ul>";
		out=(boost::format(frm) % tmp_msg % input % error).str();
	}
	return out;

}

string base_widget::render_error()
{
	if(is_set && !is_valid)
		return escape(error_msg);
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
	if(value)
		out+=" checked=\"checked\"";
	if(!id.empty())
		out+=" id=\""+id+"\"";
	if(how & as_xhtml)
		out+=" />";
	else
		out+=" >";
	return out;
}

string checkbox::render_error()
{
	return "";
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
	return boost::regex_match(value,exp);
}

boost::regex email::exp_email("^[^@]+@[^@]+$");

void select::add(string v,string opt)
{
	option o;
	o.value=v;
	o.option=opt;
	select_list.push_back(o);
}

void select::add(int val,string opt)
{
	add((boost::format("%d") % val).str(),opt);
}

void select::set(string v)
{
	value=v;
}

void select::set(int v)
{
	value=(boost::format("%d") % v).str();
}

string select::get()
{
	return value;
}

int select::geti()
{
	return atoi(value.c_str());
}

string select::render_input(int how)
{
	string out="<select name=\"";
	out+=name;
	out+="\" ";
	if(!id.empty())
		out+="id=\""+id+"\" ";
	out+=">\n";
	for(list<option>::iterator p=select_list.begin(),e=select_list.end();p!=e;++p) {
		out+="<option value=\""+p->value+"\" ";
		if(p->value==value)
			out+=" selected=\"selected\" ";
		out+=">";
		out+=escape(p->option);
		out+="</option>\n";
	}
	out+="</select>\n";
	return out;
}

void select::load(cgicc::Cgicc const &cgi)
{
	cgicc::const_form_iterator p=cgi.getElement(name);
	if(p==cgi.getElements().end()) {
		is_set=false;
	}
	else {
		value=**p;
	}
}

bool select::validate()
{
	if(!is_set) {
		is_valid=false;
		return false;
	}
	for(list<option>::iterator p=select_list.begin(),e=select_list.end();p!=e;++p) {
		if(value==p->value)
			is_valid=true;
		return true;
	}
	is_valid=false;
	return false;
}

string hidden::render(int how)
{
	string out="<input type=\"hidden\" name=\"";
	out+=name;
	out+="\" value=\"";
	out+=value;
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

} // Namespace widgets 

} // namespace cppcms
