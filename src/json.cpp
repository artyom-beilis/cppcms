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
#include <cppcms/json.h>
#include <sstream>
#include <stdio.h>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <stack>
#include "utf_iterator.h"

#include <cppcms/config.h>
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/variant.hpp>
#else // Internal Boost
#   include <cppcms_boost/variant.hpp>
    namespace boost = cppcms_boost;
#endif

namespace cppcms {
namespace json {

	bad_value_cast::bad_value_cast() : msg_("cppcms::json::bad_cast") 
	{
	}
	bad_value_cast::bad_value_cast(std::string const &s) : msg_("cppcms::json::bad_cast: "+s ) 
	{
	}
	bad_value_cast::bad_value_cast(std::string const &s,json_type actual) : 
		msg_("cppcms::json::bad_cast: ")
	{
		std::ostringstream msg;
		msg<<"error converting from "<<actual;
		msg_ +=msg.str();

	}
	bad_value_cast::bad_value_cast(std::string const &s,json_type expected, json_type actual) : 
		msg_("cppcms::json::bad_cast: ")
	{
		std::ostringstream msg;
		msg<<"error converting from "<<actual<<" to "<<expected;
		msg_ +=msg.str();

	}
	bad_value_cast::~bad_value_cast() throw()
	{
	}
	const char* bad_value_cast::what() const throw()
	{
		return msg_.c_str();
	}

	typedef boost::variant<
			undefined,
			null,
			bool,
			double,
			std::string,
			json::object,
			json::array
		>  variant_type;

	struct value::_data {
	public:
		variant_type &value()
		{
			return value_;			
		}

		variant_type const &value() const
		{
			return value_;
			
		}
	private:
		variant_type value_;
	};
	using namespace std;
	
	value::copyable::copyable() :
		d(new value::_data()) 
	{
	}
	value::copyable::~copyable()
	{
	}
	value::copyable::copyable(copyable const &r) : 
		d(r.d) 
	{
	}
	value::copyable const &value::copyable::operator=(value::copyable const &r)
	{
		if(&r!=this)
			d=r.d;
		return *this;
	}
	
	template<typename T>
	T &get_variant_value(variant_type &v)
	{
		try {
			return boost::get<T>(v);
		}
		catch(boost::bad_get const &e) {
			throw bad_value_cast("",json_type(v.which()));
		}
	}
	template<typename T>
	T const &get_variant_value(variant_type const &v)
	{
		try {
			return boost::get<T>(v);
		}
		catch(boost::bad_get const &e) {
			throw bad_value_cast("",json_type(v.which()));
		}
	}

	double const &value::number() const
	{
		return get_variant_value<double>(d->value());
	}
	bool const &value::boolean() const
	{
		return get_variant_value<bool>(d->value());
	}
	std::string const &value::str() const
	{
		return get_variant_value<std::string>(d->value());
	}

	json::object const &value::object() const
	{
		return get_variant_value<json::object>(d->value());
	}
	json::array const &value::array() const
	{
		return get_variant_value<json::array>(d->value());
	}
	double &value::number() 
	{
		return get_variant_value<double>(d->value());
	}
	bool &value::boolean()
	{
		return get_variant_value<bool>(d->value());
	}
	std::string &value::str()
	{
		return get_variant_value<std::string>(d->value());
	}

	json::object &value::object()
	{
		return get_variant_value<json::object>(d->value());
	}
	json::array &value::array()
	{
		return get_variant_value<json::array>(d->value());
	}

	bool value::is_undefined() const
	{
		return d->value().which()==json::is_undefined;
	}
	bool value::is_null() const
	{
		return d->value().which()==json::is_undefined;
	}


	void value::undefined()
	{
		d->value()=json::undefined();
	}
	void value::null()
	{
		d->value()=json::null();
	}
	void value::boolean(bool x)
	{
		d->value()=x;
	}
	void value::number(double x)
	{
		d->value()=x;
	}
	void value::str(std::string const &x)
	{
		d->value()=x;
	}
	void value::object(json::object const &x)
	{
		d->value()=x;
	}
	void value::array(json::array const &x)
	{
		d->value()=x;
	}

	json_type value::type() const
	{
		return json_type(d->value().which());
	}

	namespace {

		std::string escape(std::string const &input)
		{
			std::string result;
			result.reserve(input.size());
			result+='"';
			std::string::const_iterator i,end=input.end();
			for(i=input.begin();i!=end;i++) {
				switch(*i) {
				case 0x22:
				case 0x5C:
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
						char buf[8];
#ifdef CPPCMS_HAVE_SNPRINTF						
						snprintf(buf,sizeof(buf),"\\u%04X",*i);
#else
						sprintf(buf,"\\u%04X",*i);
#endif
						result+=buf;
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


	void value::write(std::ostream &out,int tabs) const
	{
		std::locale original(out.getloc());
		out.imbue(std::locale("C"));
		try {
			write_value(out,tabs);
		}
		catch(...) {
			out.imbue(original);
			throw;
		}
		out.imbue(original);

	}

	void value::write_value(std::ostream &out,int tabs) const
	{
		switch(type()) {
		case json::is_undefined:
			throw bad_value_cast("Can't write undefined value to stream");
		case json::is_null:
			out<<"null";
			break;
		case json::is_number:
			out<<number();
			break;
		case json::is_string:
			out<<escape(str());
			break;
		case json::is_boolean:
			out<< (boolean() ? "true" : "false") ;
			break;
		case json::is_array:
			{
				json::array const &a=array();
				unsigned i;
				indent(out,'[',tabs);
				for(i=0;i<a.size();) {
					a[i].write_value(out,tabs);
					i++;
					if(i<a.size())
						indent(out,',',tabs);
				}
				indent(out,']',tabs);
			}
			break;
		case json::is_object:
			{
				json::object const &obj=object();
				object::const_iterator p,end;
				p=obj.begin();
				end=obj.end();
				indent(out,'{',tabs);
				while(p!=end) {
					out<<escape(p->first);
					indent(out,':',tabs);
					p->second.write_value(out,tabs);
					++p;
					if(p!=end)
						indent(out,',',tabs);
				}
				indent(out,'}',tabs);
			}
			break;
		default:
			throw bad_value_cast("Unknown type found: internal error");
		}
	}

	void value::save(std::ostream &out,int how) const
	{
		int tabs=(how & readable) ? 0 : -1;
		write(out,tabs);
	}
	
	std::string value::save(int how) const
	{
		std::ostringstream ss;
		int tabs=(how & readable) ? 0 : -1;
		write(ss,tabs);
		return ss.str();
	}

	std::ostream &operator<<(std::ostream &out,value const &v)
	{
		v.save(out);
		return out;
	}

	bool value::operator==(value const &other) const
	{
		return d->value()==other.d->value();
	}
	bool value::operator!=(value const &other) const
	{
		return !(*this==other);
	}


	// returns empty if not found
	value const &value::find(std::string path) const
	{
		static value const empty;
		value const *ptr=this;
		size_t pos=0;
		size_t new_pos;
		do {
			new_pos=path.find('.',pos);
			std::string part=path.substr(pos,new_pos - pos);
			if(new_pos!=std::string::npos)
				new_pos++;
			if(part.empty())
				return empty;
			if(ptr->type()!=json::is_object)
				return empty;
			json::object const &obj=ptr->object();
			json::object::const_iterator p;
			if((p=obj.find(part))==obj.end())
				return empty;
			ptr=&p->second;
			pos=new_pos;

		} while(new_pos < path.size());
		return *ptr;
	}
	
	// throws if not found
	value const &value::at(std::string path) const
	{
		value const &v=find(path);
		if(v.is_undefined())
			throw bad_value_cast("Value not found at "+path );
		return v;
	}
	value &value::at(std::string path)
	{
		value *ptr=this;
		size_t pos=0;
		size_t new_pos;
		do {
			new_pos=path.find('.',pos);
			std::string part=path.substr(pos,new_pos - pos);
			if(new_pos!=std::string::npos)
				new_pos++;
			if(part.empty())
				throw bad_value_cast("Invalid path provided");
			if(ptr->type()!=json::is_object)
				throw bad_value_cast("",ptr->type(),json::is_object);
			json::object &obj=ptr->object();
			json::object::iterator p;
			if((p=obj.find(part))==obj.end())
				throw bad_value_cast("Member "+part+" not found");
			ptr=&p->second;
			pos=new_pos;

		} while(new_pos < path.size());
		return *ptr;
	}
	void value::at(std::string path,value const &v)
	{
		value *ptr=this;
		size_t pos=0;
		size_t new_pos;
		do {
			new_pos=path.find('.',pos);
			std::string part=path.substr(pos,new_pos - pos);


			if(new_pos!=std::string::npos)
				new_pos++;
			if(part.empty())
				throw bad_value_cast("Invalid path provided");
			if(ptr->type()!=json::is_object) {
				*ptr=json::object();
			}
			json::object &obj=ptr->object();
			json::object::iterator p;
			if((p=obj.find(part))==obj.end()) {
				ptr=&obj.insert(std::make_pair(part,json::value())).first->second;
			}
			else
				ptr=&p->second;
			pos=new_pos;

		} while(new_pos < path.size());
		*ptr=v;
	}

	value &value::operator[](std::string name)
	{
		if(type()!=json::is_object)
			set_value(json::object());

		json::object &self=object();

		json::object::iterator p=self.find(name);
		if(p==self.end())
			return self.insert(std::make_pair(name,value())).first->second;
		return p->second;
	}

	value const &value::operator[](std::string name) const
	{
		if(type()!=json::is_object)
			throw bad_value_cast("",type(),json::is_object);
		json::object const &self=object();
		json::object::const_iterator p=self.find(name);
		if(p==self.end())
			throw bad_value_cast("Member "+name+" not found");
		return p->second;
	}

	value &value::operator[](size_t n)
	{
		if(type()!=json::is_array)
			set_value(json::array());
		json::array &self=array();
		if(n>=self.size())
			self.resize(n+1,json::null());
		return self[n];
	}
	
	value const &value::operator[](size_t n) const
	{
		if(type()!=json::is_array)
			throw bad_value_cast("",type(),json::is_array);
		if(n >= array().size())
			throw bad_value_cast("Index out of range");
		return array()[n];
	}

//#define DEBUG_PARSER


	namespace {

		enum {	
			tock_eof = 255,
			tock_err,
			tock_str,
			tock_number,
			tock_true,
			tock_false,
			tock_null 
		};


		struct tockenizer {
		public:
			tockenizer(std::istream &in) : 
				real(0),
				line(1),
				is_(in),
				locale_(in.getloc())
			{
				is_.imbue(std::locale::classic());
			}
			~tockenizer()
			{
				is_.imbue(locale_);
			}

			std::string str;
			double real;
			int line;
#ifdef DEBUG_PARSER
			std::string write_tocken(int c)
			{
				std::ostringstream ss;
				if(c>=255) {
					static char const *name[] = {
						"eof",
						"err",
						"str",
						"number",
						"true",
						"false",
						"null"
					};
					ss<<name[c - 255];
				}
				else {
					ss<<char(c);
				}
				if(c==tock_str)
					ss<<" "<<str;
				else if(c==tock_number)
					ss<<" "<<real;
				return ss.str();

			}
#endif

			int next()
			{
				for(;;) {
					char c;
					if(!is_.get(c))
						return tock_eof;
					
					switch(c) {
					case '[':
					case '{':
					case ':':
					case ',':
					case '}':
					case ']':
						return c;
					case ' ':
					case '\t':
					case '\r':
						break;
					case '\n':
						line++;
						break;
					case '"':
						is_.unget();
						if(parse_string())
							return tock_str;
						return tock_err;
					case 't':
						if(check("rue"))
							return tock_true;
						return tock_err;
					case 'n':
						if(check("ull"))
							return tock_null;
						return tock_err;
					case 'f':
						if(check("alse"))
							return tock_false;
						return tock_err;
					case '-':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						is_.unget();
						if(parse_number())
							return tock_number;
						return tock_err;
					case '/':
						if(check("/")) {
							while(is_.get(c) && c!='\n')
								;
							if(c=='\n')
								break;
							return tock_eof;
						}
						return tock_err;
					default:
						return tock_err;
					}
				}
			}
		private:
			std::istream &is_;
			std::locale locale_;

			bool check(char const *s)
			{
				char c;
				while(*s && is_.get(c) && c==*s)
					s++;
				return *s==0;
			}
			bool parse_string()
			{
				char c;
				str.clear();
				if(!is_.get(c) || c!='"')
					return false;
				bool second_surragate_expected=false;
				uint16_t first_surragate = 0;
				for(;;) {
					if(!is_.get(c))
						return false;
					if(second_surragate_expected && c!='\\')
						return false;
					if(0<= c && c <= 0x1F)
						return false;
					if(c=='"')
						break;
					if(c=='\\') {
						if(!is_.get(c))
							return false;
						if(second_surragate_expected && c!='u')
							return false;

						switch(c) {
						case	'"':
						case	'\\':
						case	'/':
							str+=char(c);
							break;

						case	'b': str+='\b'; break;
						case	'f': str+='\f'; break;
						case	'n': str+='\n'; break;
						case	'r': str+='\r'; break; 
						case	't': str+='\t'; break;
						case	'u': 
							{
								uint16_t x;
								if(!read_4_digits(x))
									return false;
								if(second_surragate_expected) {
									if(!utf16::is_second_surrogate(x))
										return false;
									append(utf16::combine_surrogate(first_surragate,x));
									second_surragate_expected=false;
								}
								else if(utf16::is_first_surrogate(x)) {
									second_surragate_expected=true;
									first_surragate=x;
								}
								else {
									append(x);
								}
								
							}
							break;
						default:
							return false;
						}
					}
					else {
						str+=char(c);
					}
				}
				if(!utf8::validate(str.begin(),str.end()))
					return false;
				return true;
			}

			bool read_4_digits(uint16_t &x)
			{
				char buf[5]={0};
				if(!is_.get(buf,5))
					return false;
				for(unsigned i=0;i<4;i++) {

					char c=buf[i];

					if(	('0'<= c && c<='9')
						|| ('A'<= c && c<='F')
						|| ('a'<= c && c<='f') )
					{
						continue;
					}
					return false;
				}
				unsigned v;
				sscanf(buf,"%x",&v);
				x=v;
				return true;
			}

			void append(uint32_t x)
			{
				utf8::seq s=utf8::encode(x);
				str.append(s.c,s.len);
			}


			bool parse_number()
			{
				is_ >> real;
				return is_;
			}
			
		};

		typedef enum {
			st_init = 0,
			st_object_or_array_or_value_expected = 0 ,
			st_object_key_or_close_expected,
			st_object_colon_expected,
			st_object_value_expected,
			st_object_close_or_comma_expected,
			st_array_value_or_close_expected,
			st_array_close_or_comma_expected,
			st_error,
			st_done
		} state_type;

#ifdef DEBUG_PARSER
		std::ostream &operator<<(std::ostream &out,state_type t)
		{
			static char const *names[] = {
				"st_object_or_array_or_value_expected",
				"st_object_key_or_close_expected",
				"st_object_colon_expected",
				"st_object_value_expected",
				"st_object_close_or_comma_expected",
				"st_array_value_or_close_expected",
				"st_array_close_or_comma_expected",
				"st_error",
				"st_done"
			};
			out<<names[t];
			return out;
		}
#endif

		bool parse_stream(std::istream &in,value &out,bool force_eof,int &error_at_line)
		{
			tockenizer tock(in);
			value result;
		
			state_type state=st_init;
			
			std::string key;

			std::stack<std::pair<state_type,value *> > stack;

			stack.push(std::make_pair(st_done,&result));

			while(!stack.empty() && state !=st_error && state!=st_done) {

				int c=tock.next();
#ifdef DEBUG_PARSER
				std::cout<<state<<" "<<tock.write_tocken(c)<<std::endl;
#endif

				switch(state) {
				case st_object_or_array_or_value_expected:
					if(c=='[')  {
						*stack.top().second=json::array();
						state=st_array_value_or_close_expected;
					}
					else if(c=='{') {
						*stack.top().second=json::object();
						state=st_object_key_or_close_expected;
					}
					else if(c==tock_str)  {
						*stack.top().second=tock.str; 
						state=stack.top().first;
						stack.pop();
					}
					else if(c==tock_true) {
						*stack.top().second=true;
						state=stack.top().first;
						stack.pop();
					}
					else if(c==tock_false) {
						*stack.top().second=false;
						state=stack.top().first;
						stack.pop();
					}
					else if(c==tock_null) {
						*stack.top().second=null();
						state=stack.top().first;
						stack.pop();
					}
					else if(c==tock_number) {
						*stack.top().second=tock.real;
						state=stack.top().first;
						stack.pop();
					}
					else
						state = st_error;
					break;

				case st_object_key_or_close_expected:
					if(c=='}') {
						state=stack.top().first;
						stack.pop();
					}
					else if (c==tock_str) {
						key=tock.str;
						state = st_object_colon_expected;
					}
					else
						state = st_error;
					break;
				case st_object_colon_expected:
					if(c!=':')
						state=st_error;
					else
						state=st_object_value_expected;
					break;
				case st_object_value_expected:
					{
						json::object &obj = stack.top().second->object();
						std::pair<json::object::iterator,bool> res=
							obj.insert(std::make_pair(key,json::value()));

						if(res.second==false) {
							state=st_error;
							break;
						}
						json::value &val=res.first->second;

						if(c==tock_str) { 
							val=tock.str;
							state=st_object_close_or_comma_expected;
						}
						else if(c==tock_true) {
							val=true;
							state=st_object_close_or_comma_expected;
						}
						else if(c==tock_false) {
							val=false;
							state=st_object_close_or_comma_expected;
						}
						else if(c==tock_null) {
							val=null();
							state=st_object_close_or_comma_expected;
						}
						else if(c==tock_number) {
							val=tock.real;
							state=st_object_close_or_comma_expected;
						}
						else if(c=='[') {
							val=json::array();
							state=st_array_value_or_close_expected;
							stack.push(std::make_pair(st_object_close_or_comma_expected,&val));
						}
						else if(c=='{') {
							val=json::object();
							state=st_object_key_or_close_expected;
							stack.push(std::make_pair(st_object_close_or_comma_expected,&val));
						}
						else 
							state=st_error;
					}
					break;
				case st_object_close_or_comma_expected:
					if(c==',')
						state=st_object_key_or_close_expected;
					else if(c=='}') {
						state=stack.top().first;
						stack.pop();
					}
					else
						state=st_error;
					break;
				case st_array_value_or_close_expected:
					{
						if(c==']') {
							state=stack.top().first;
							stack.pop();
							break;
						}

						json::array &ar = stack.top().second->array();
						ar.push_back(json::value());
						json::value &val=ar.back();

						if(c==tock_str)  {
							val=tock.str; 
							state=st_array_close_or_comma_expected;
						}
						else if(c==tock_true) {
							val=true;
							state=st_array_close_or_comma_expected;
						}
						else if(c==tock_false) {
							val=false;
							state=st_array_close_or_comma_expected;
						}
						else if(c==tock_null) {
							val=null();
							state=st_array_close_or_comma_expected;
						}
						else if(c==tock_number) {
							val=tock.real;
							state=st_array_close_or_comma_expected;
						}
						else if(c=='[') {
							val=json::array();
							state=st_array_value_or_close_expected;
							stack.push(std::make_pair(st_array_close_or_comma_expected,&val));
						}
						else if(c=='{') {
							val=json::object();
							state=st_object_key_or_close_expected;
							stack.push(std::make_pair(st_array_close_or_comma_expected,&val));
						}
						else 
							state=st_error;
						break;
					}
				case st_array_close_or_comma_expected:
					if(c==']') {
						state=stack.top().first;
						stack.pop();
					}
					else if(c==',')
						state=st_array_value_or_close_expected;
					else
						state=st_error;
					break;
				case st_done:
				case st_error:
					break;

				};
			}
			if(state==st_done) { 
				if(force_eof) {
					if(tock.next()!=tock_eof) {
						error_at_line=tock.line;
						return false;
					}
				}
				out.swap(result);
				return true;
			}
			error_at_line=tock.line;
			return false;

		}
	} // anonymous

	bool value::load(std::istream &in,bool full,int *line_number)
	{
		int err_line;
		value v;
		if(!parse_stream(in,*this,full,err_line)) {
			if(line_number)
				*line_number=err_line;
			return false;
		}
		return true;

	}
	
	std::istream &operator>>(std::istream &in,value &v)
	{
		int line_no;
		if(!parse_stream(in,v,false,line_no))
			in.setstate( std::istream::failbit );
		return in;
	}
	
	std::ostream &operator<<(std::ostream &out,json_type t)
	{
		switch(t) {
		case is_undefined: out<<"undefined"; break;
		case is_null: out<<"null"; break;
		case is_boolean: out<<"boolean"; break;
		case is_number: out<<"number"; break;
		case is_string: out<<"string"; break;
		case is_object: out<<"object"; break;
		case is_array: out<<"array"; break;
		default:
			out<<"Illegal";
		}
		return out;
	}



} // json

} // cppcms
