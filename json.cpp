#include "json.h"
#include "encoding.h"
#include <sstream>
#include <stdio.h>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include <stdexcept>


namespace cppcms {
namespace json {

	struct value_data {
		value_data() : type(is_null) {}
		json_type type;
		std::string str_;
		object obj_;
		array arr_;
		double num_;
		bool bool_;
		void clear() {
			switch(type) {
			case is_string: str_.clear(); break;
			case is_object: obj_.clear(); break;
			case is_array: arr_.clear(); break;
			default: ;
			}
			type=is_null;
		}
		void set(bool v) { clear() ; type=is_boolean; bool_=v; }
		void set(double v) { clear() ; type=is_number; num_=v; }
		void set(std::string v) { clear() ; type=is_string; str_=v; }
		void set(object const &v) { clear() ; type=is_object; obj_=v; }
		void set(array const &v) { clear() ; type=is_array; arr_=v; }
	};

	bool value::is_null() const
	{
		return d->type==json::is_null;
	}
	void value::null()
	{
		d->clear();
	}
	double value::real() const
	{
		if(d->type==is_number)
			return d->num_;
		throw std::bad_cast();
	}
	double value::real(double def) const
	{
		if(d->type==is_number)
			return d->num_;
		else if(d->type==json::is_null)
			return def;
		throw std::bad_cast();
	}
	int value::integer() const
	{
		double real_value=real();
		int value=static_cast<int>(real_value);
		if(value!=real_value)
			throw std::bad_cast();
		return value;
	}
	int value::integer(int def) const
	{
		double real_value=real(def);
		int value=static_cast<int>(real_value);
		if(value!=real_value)
			throw std::bad_cast();
		return value;
	}
	bool value::boolean() const
	{
		if(d->type==is_boolean)
			return d->bool_;
		throw std::bad_cast();
	}
	bool value::boolean(bool def) const
	{
		if(d->type==is_boolean)
			return d->bool_;
		if(d->type==json::is_null)
			return def;
		throw std::bad_cast();
	}
	std::string value::str() const
	{
		if(d->type==is_string)
			return d->str_;
		throw std::bad_cast();
	}
	std::string value::str(std::string def) const
	{
		if(d->type==is_string)
			return d->str_;
		if(d->type==json::is_null)
			return def;
		throw std::bad_cast();
	}

	std::wstring value::wstr() const
	{
		return encoding::to_wstring(str());
	}
	std::wstring value::wstr(std::wstring def) const
	{
		if(d->type==json::is_null)
			return def;
		return encoding::to_wstring(str());
	}

	json::object const &value::object() const
	{
		if(d->type==is_object)
			return d->obj_;
		throw std::bad_cast();
	}
	json::object &value::object()
	{
		if(d->type==is_object)
			return d->obj_;
		throw std::bad_cast();
	}
	json::array const &value::array() const
	{
		if(d->type==is_array)
			return d->arr_;
		throw std::bad_cast();
	}
	json::array &value::array()
	{
		if(d->type==is_array)
			return d->arr_;
		throw std::bad_cast();
	}

	value const &value::operator=(double v)
	{
		d->set(v);
		return *this;
	}
	value const &value::operator=(bool v)
	{
		d->set(v);
		return *this;
	}
	value const &value::operator=(int v)
	{
		d->set(double(v));
		return *this;
	}
	value const &value::operator=(std::string v)
	{
		d->set(v);
		return *this;
	}
	value const &value::operator=(std::wstring v)
	{
		d->set(encoding::to_string(v));
		return *this;
	}
	value const &value::operator=(char const *v)
	{
		d->set(std::string(v));
		return *this;
	}
	value const &value::operator=(wchar_t const *v)
	{
		d->set(encoding::to_string(v));
		return *this;
	}
	value const &value::operator=(json::object const &v)
	{
		d->set(v); return *this;
	}
	value const &value::operator=(json::array const &v)
	{
		d->set(v); return *this;
	}

	value &value::operator[](unsigned pos)
	{
		if(is_null())
			d->set(json::array());
		if(d->type!=is_array)
			throw std::bad_cast();
		if(d->arr_.size()<=pos)
			d->arr_.resize(pos+1);
		return d->arr_[pos];
	}
	value const &value::operator[](unsigned pos) const
	{
		static value null;
		if(is_null())
			return null;
		if(d->type!=is_array)
			throw std::bad_cast();
		if(d->arr_.size()<=pos)
			return null;
		return d->arr_[pos];
	}
	value const &value::operator()(std::string const &path) const
	{
		static const value null;
		if(is_null())
			return null;
		json::object const &members=object();
		json::object::const_iterator p;
		std::string::const_iterator i=find(path.begin(),path.end(),'.');
		if(i==path.end()) {
			if((p=members.find(path))!=members.end())
				return p->second;
			return null;
		}
		std::string prefix(path.begin(),i);
		value const &tmp=(*this)[prefix];
		if(tmp.is_null())
			return null;
		return tmp[std::string(i+1,path.end())];
	}
	value &value::operator()(std::string const &path)
	{
		if(is_null())
			d->set(json::object());
		json::object::iterator p;
		json::object &members=object();
		std::string::const_iterator i=find(path.begin(),path.end(),'.');
		if(i==path.end())
			return members[path];
		std::string prefix(path.begin(),i);
		value &tmp=(*this)[prefix];
		return tmp[std::string(i+1,path.end())];
	}
	value const &value::operator[](std::string const &entry) const
	{
		static const value null;
		if(is_null())
			return null;
		json::object const &members=object();
		json::object::const_iterator p;
		if((p=members.find(entry))!=members.end())
			return p->second;
		return null;
	}
	value &value::operator[](std::string const &entry)
	{
		if(is_null())
			d->set(json::object());
		json::object::iterator p;
		json::object &members=object();
		return members[entry];
	}

	value::value() : d(new value_data) { }
	value::value(value const &other) : d(other.d) {}
	value const &value::operator=(value const &other)
	{
		d=other.d;
		return *this;
	}
	value::~value()  {}


	namespace {
		std::string uchar(unsigned v)
		{
			char buf[8];
			snprintf(buf,sizeof(buf),"\\u%04X",v);
			std::string res=buf;
			return res;
		}
		std::string uchar(std::string::const_iterator p,std::string::const_iterator e)
		{

			return uchar(*p);
		}

		std::string escape(std::string const &input,bool utf)
		{
			std::string result;
			result.reserve(input.size());
			result+='"';
			std::string::const_iterator i,end=input.end();
			for(i=input.begin();i!=end;i++) {
				switch(*i) {
				case 0x22:
				case 0x5C:
				case 0x2F:
					result+='\\';
					result+=*i;
					break;
				case '\b': result+="\\b"; break;
				case '\f': result+="\\f"; break;
				case '\n': result+="\\n"; break;
				case '\r': result+="\\r"; break;
				case '\t': result+="\\t"; break;
				default:
					if(0x00<=*i && *i<=0x1F) {
						uchar(*i);
					}
					else if((*i & 0x80) && !utf) {
						result+=uchar(i,end);
					}
					else {
						result+=*i;
					}
				}
			}
			result+='"';
			return result;
		}

		void pad(std::ostream &out,int tb)
		{
			for(;tb > 0;tb--) out<<'\t';
		}

		void indent(std::ostream &out,char c,int &tabs)
		{
			if(tabs < 0) {
				out<<c;
				return;
			}
			switch(c) {
			case '{':
			case '[':
				out<<c<<'\n';
				tabs++;
				pad(out,tabs);
				break;
			case ',':
				out<<c<<'\n';
				pad(out,tabs);
				break;
			case ':':
				out<<" :\t";
				break;
			case '}':
			case ']':
				out<<'\n';
				tabs--;
				pad(out,tabs);
				out<<c<<'\n';
				pad(out,tabs);
				break;
			}
		}

	} // namespace anonymous


	json_type value::type() const
	{
		return d->type;
	}

	void value::write(std::ostream &out,int tabs,bool utf) const
	{
		if(is_null()) {
			out<<"null";
			return;
		}
		switch(d->type) {
		case is_number:
			out<<d->num_;
			break;
		case is_string:
			out<<escape((d->str_),utf);
			break;
		case is_boolean:
			out<< (d->bool_ ? "true" : "false") ;
			break;
		case is_array:
			{
				json::array const &a=d->arr_;
				if(!a.empty()) {
					unsigned i;
					indent(out,'[',tabs);
					for(i=0;i<a.size();) {
						a[i].write(out,tabs,utf);
						i++;
						if(i<a.size())
							indent(out,',',tabs);
					}
					indent(out,']',tabs);
				}
			}
			break;
		case is_object:
			{
				json::object const &obj=d->obj_;
				object::const_iterator p,end;
				p=obj.begin();
				end=obj.end();
				if(p!=end) {
					indent(out,'{',tabs);
					while(p!=end) {
						out<<escape(p->first,utf);
						indent(out,':',tabs);
						p->second.write(out,tabs,utf);
						++p;
						if(p!=end)
							indent(out,',',tabs);
					}
					indent(out,'}',tabs);
				}
			}
			break;
		default:
			throw std::bad_cast();
		}
	}

	std::string value::save(int how) const
	{
		std::ostringstream ss;
		ss.imbue(std::locale("C"));
		bool utf=(how & us_ascii) == 0;
		int tabs=(how & readable) ? 0 : -1;
		write(ss,tabs,utf);
		return ss.str();
	}

	std::ostream &operator<<(std::ostream &out,value const &v)
	{
		out<<v.save();
		return out;
	}

	bool value::operator==(value const &other) const
	{
		if(d->type!=other.d->type)
			return false;
		switch(d->type) {
		case json::is_null: return true;
		case is_number: return d->num_==other.d->num_;
		case is_string: return d->str_==other.d->str_;
		case is_boolean: return d->bool_==other.d->bool_;
		case is_object: return d->obj_==other.d->obj_;
		case is_array: return d->arr_==other.d->arr_;
		default: return false;
		}
	}
	bool value::operator!=(value const &other) const
	{
		return !(*this == other);
	}
/*
	namespace {

	enum {	tock_eof = 256,
		tock_err,
		tock_str,
		tock_bool,
		tock_null,
		tock_num }

	std::pair<int,value> tocken(int c)
	{
		return std::pair<int,value>(c,value());
	}
	std::pair<int,value> tocken(int c,value const &v)
	{
		return std::pair<int,value>(c,v);
	}


	std::pair<int,value> check_string(std::string::const_iterator &p,std::string::const_iterator e)
	{
		std::string result;
		bool prev_surragate=false;
		uint16_t upair[2];
		for(;p!=e;) {
			std::string::const_iterator prev=p;
			uint32_t c=utf8::next(p,e,false,true); // Decode, ignore special
			if(prev_surragate){
				if(!(c=='\\' && p+1!=e && *(p+1)=='u'))
					return tocken(tock_err);
			}
			if(c==utf::illegal)
				return tocken(tock_err);
			else if(c=='"'){
				value v=result;
				return tocken(tock_str,v);
			}
			else if(c=='\\') {
				if(p==e) return tocken(tock_err);
				c=(unsigned char)*p++;
				switch(c) {
				case '\\': result+='\\'; break;
				case '/': result+='/'; break;
				case 'n': result+='\n'; break;
				case 'r': result+='\r'; break;
				case 't': result+='\t'; break;
				case 'f': result+='\f'; break;
				case 'b': result+='\b'; break;
				case 'u':
					{
						uint16_t x=0;
						for(int n=0;n<4;n++) {
							if(p!=e) return tocken(tock_err);
							c=*p++;
							if('0'<=c && c<='9')
								x=(x<<4) | (c-'0')
							else if('a'<=c && c<='f')
								x=(x<<4) | (c-'a'+10)
							else if('A'<=c && c<='F')
								x=(x<<4) | (c-'A'+10)
							else
								return tocken(tock_err);
						}
						if(prev_surragate && 0xDC00 <=x && x<=0xDFFF) {
							upair[1]=x;
							uint16_t *ptr=upair;
							prev_surragate=false;
							result+=utf8::encode(utf16::next(ptr,ptr+2)).c;
						}
						else if(prev_surragate)
							return tocken(tock_err);
						else if(0xD800 <= x && x<=0xDBFF) {
							prev_surragate=true;
							upair[0]=x;
						}
						else if(0xDC00 <=x && x<=0xDFFF)
							return tocken(tock_err);
						else if(x==0)
							result+='\0';
						else 
							result+=utf8::encode(x).c;
						break;

					}
				case '"': result+='"'; break;
				default: return tocken(tock_err);
				}
			}
			else {
				result.append(prev,p);
			}
		}
	}
	std::pair<int,value> check_number(std::string::const_iterator &p,std::string::const_iterator e)
	{
		enum {	start,
			minus_found,
			diggit_found,
			point_found,
			exp_found,
			exp_sign_found,
			exp_digit_found
			stop,
			syntax}
		state=start;
		std::string::const_iterator begin=p;
		
		for(;state!=stop && state!=syntax && p!=e;) {
			c=*p++;
			switch(state) {
			case start:
				if(c=='-')
					stat=minus_found;
				else if('1'<=c && c<='9')
					state=diggit_found;
				else if(c=='0')
					state=stop;
				else
					state=syntax;
				break;
			case minus_found:
				if('1'<=c && c<='9')
					state=digit_found;
				else if(c=='.')
					state=point_found;
				else
					state=syntax;
				break;
			case digit_found:
				if('0'<=c && c<='9')
					;
				else if(c=='.')
					state=point_found;
				else if(c=='e' || c=='E')
					state=exp_found;
				else
					state=stop;
				break;
			case point_found:
				if('0'<=c && c<='9')
					;
				else if(c=='e' || c=='E')
					state=exp_found;
				else
					state=stop;
				break;
			case exp_found:
				if(c=='-' || c=='+')
					state=exp_sign_found;
				else if('1'<=c && c<='9')
					state=exp_digit_found;
				else
					state=syntax;
				break;
			case exp_sign_found:
				if('1'<=c && c<='9')
					state=exp_digit_found;
				else
					state=syntax;
				break;
			case exp_digit_found:
				if('0'<=c && c<='9')
					;
				else
					state=stop;
			default:
				throw std::runtime_error("Internal state error");:			
			}
		}
		if(state==syntax || state==minus_found || state==exp_found || state==exp_sign_found)
			return tocken(tock_err);
		if(p!=e)
			p--;
		std::string num(begin,p);
		value v=atof(num.c_str());
		return tocken(tock_num,v);
	}


	std::pair<int,value> next_tocken(std::string::const_iterator &p,std::string::const_iterator e,int &lines)
	{
		if(p==e) {
			return tocken(tock_eof);
		}
		while(p!=e) {
			char c=*p++;
			switch(c) {
			case	'/':
				if(p!=e && *p=='/') {
					while(p!=e && '\n'!=*p++)
						;
					break;
				}
				return tocken(tock_err);

			case	'\r':
				lines++;
				// Fall
			case	' ':
			case	'\t':
			case	'\n':	
			case	'\f':

				break;

			case	'[':
			case	'{':
			case	']':
			case	'}':
			case	',':
			case	':':

				return tocken(c);
			case	't':
				if(	p!=e && 'r'==*p++
					&& p!=e && 'u'==*p++
					&& p!=e && 'e'==*p++)
				{
					value v=true;
					return tocken(tock_bool,v);
				}
				return tocken(tock_err);
			case	'f':
				if(	p!=e && 'a'==*p++
					&& p!=e && 'l'==*p++
					&& p!=e && 's'==*p++
					&& p!=e && 'e'==*p++)
				{
					value v=false;
					return tocken(tock_bool,v);
				}
				return tocken(error);
			case	'n':
				if(	p!=e && 'u'==*p++
					&& p!=e && 'l'==*p++
					&& p!=e && 'l'==*p++)
				{
					return tocken(tock_null);
				}
				return tocken(tock_err);
			case	'"':
				return check_string(p,e);
			case	'-':
			case	'0':
			case	'1':
			case	'2':
			case	'3':
			case	'4':
			case	'5':
			case	'6':
			case	'7':
			case	'8':
			case	'9':
				p--;
				return check_number(p,e);
			default:
				return tocken(tock_err);
			}
		}
		return tocken(tock_eof);
	}

	} // anon namespace
	int value::load(std::string const &s)
	{
		lines=0;
		std::string::const_iterator p=s.begin(),e=s.end();
		stack<value> st;
		enum {
			idle,
			arr_open_found,
			obj_open_found,
			arr_comma_expected,
			obj_col_expected,
			obj_value_expected,
			obj_comma_expected,
			end_of_obj_found,
			error
		};

		}
		int state=idle;
		value curr;
		for(;;) {
			std::pair<int,value> tock=next_tocken(p,e);
			int tid;
			switch(state) {
			case idle:
				switch(tid) {
				case '[': state=arr_open_found; curr=array(); break;
				case '{': state=obj_open_found; curr=object(); break;
				default: state=error; break;
				}
				break;
			case arr_open_found:
				switch(tid) {
				case tock_num:
				case tock_str:
				case tock_bool:
				case tock_null:
					curr.array().push_back(tock.second);
					state=arr_comma_expected;
					break;
				default: state=error; break;
				}
				break;
			case arr_comma_expected:
				if(tid==',') { state=arr_open_found; break; }
				// TODO
			case obj_open_found:
				if(tid!=tock_str) { state=error; break; }
				str=tock.second.str();
				state=obj_comma_expected;
				break;
			case 
			}
		}
	}

*/

	int value::load(std::string const &s)
	{
		throw std::runtime_error("Unsupported");
	}

} // json

} // cppcms
