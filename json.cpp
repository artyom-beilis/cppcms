#define CPPCMS_SOURCE
#include "json.h"
#include <sstream>
#include <stdio.h>
#include <typeinfo>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <boost/variant.hpp>

namespace cppcms {
namespace json {


	typedef boost::variant<
			undefined,
			null,
			bool,
			double,
			std::string,
			json::object,
			json::array
		>  variant_type;

	struct value::data {
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
		d(new value::data()) 
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
			throw std::bad_cast();
		}
	}
	template<typename T>
	T const &get_variant_value(variant_type const &v)
	{
		try {
			return boost::get<T>(v);
		}
		catch(boost::bad_get const &e) {
			throw std::bad_cast();
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
						char buf[8];
						snprintf(buf,sizeof(buf),"\\u%04X",*i);
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
			throw std::bad_cast();
		case json::is_null:
			out<<"null";
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
			throw std::bad_cast();
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
			throw std::bad_cast();
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
				throw std::bad_cast();
			if(ptr->type()!=json::is_object)
				throw std::bad_cast();
			json::object &obj=ptr->object();
			json::object::iterator p;
			if((p=obj.find(part))==obj.end())
				throw std::bad_cast();
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
				throw std::bad_cast();
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
			throw std::bad_cast();
		json::object const &self=object();
		json::object::const_iterator p=self.find(name);
		if(p==self.end())
			throw std::bad_cast();
		return p->second;
	}

	value &value::operator[](size_t n)
	{
		if(type()!=json::is_array)
			set_value(json::array());
		json::array &self=array();
		if(n>=self.size())
			self.resize(n+1);
		return self[n];
	}
	
	value const &value::operator[](size_t n) const
	{
		if(type()!=json::is_array)
			throw std::bad_cast();
		return array().at(n);
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


} // json

} // cppcms
