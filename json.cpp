#include "json.h"
#include <stdio.h>

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
		return uni::to_wstring(str());
	}
	std::wstring value::wstr(std::wstring def) const
	{
		if(d->type==json::is_null)
			return def;
		return uni::to_wstring(str());
	}

	json::object const &value::object() const
	{
		if(d->type==is_object)
			return d->obj_;
		throw std::bad_cast();
	}
	json::object &object()
	{
		if(d->type==is_object)
			return d->obj_;
		throw std::bad_cast();
	}
	json::array const &array() const
	{
		if(d->type==is_array)
			return d->array_;
		throw std::bad_cast();
	}
	json::array &array()
	{
		if(d->type==is_array)
			return d->array_;
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
		d->set(uni::wstr_to_str(v));
		return *this;
	}
	value const &value::operator=(char const *v)
	{
		d->set(std::string(v));
		return *this;
	}
	value const &value::operator=(wchar_t const *v)
	{
		d->set(uni::wstr_to_str(v));
		return *this;
	}
	value const &value::operator=(object const &v)
	{
		d->set(v); return *this;
	}
	value const &value::operator=(array const &v)
	{
		d->set(v); return *this;
	}

	value &operator[](unsigned pos)
	{
		if(d->type!=is_array && d->type!=json::is_null)
			throw std::bad_cast();
		if(d->type==json::is_null) {
			d->set(array());
		}
		if(d->arr_.size()=<pos)
			d->arr_.resize(pos+1);
		return d->arr_[pos];
	}
	value const &operator[](unsigned pos) const
	{
		if(d->type!=is_array)
			throw std::bad_cast();
		return d->arr_.at(pos);
	}
	value const &value::operator()(std::string const &path) const 
	{
		object const &members=object();
		static const value null;
		object::const_iterator p;
		std::string::const_iterator i=find(path.begin(),path.end(),'.');
		if(i==path.end()) {
			if((p=members.find(path))!=members.end())
				return p->second;
			return null;
		}
		std::string prefix(path.begin(),i);
		value const &tmp=(*this)[prefix];
		if(tmp.null()) 
			return null;
		return tmp[std::string(i+1,path.end())];
	}
	value &value::operator()(std::string const &path)
	{
		object::iterator p;
		object &members=object();
		std::string::const_iterator i=find(path.begin(),path.end(),'.');
		if(i==path.end())
			return members[path];
		std::string prefix(path.begin(),i);
		value &tmp=(*this)[prefix];
		return tmp[std::string(i+1,path.end())];
	}
	value const &value::operator[](std::string const &entry) const 
	{
		object const &members=object();
		static const value null;
		object::const_iterator p;
		if((p=members.find(path))!=members.end())
			return p->second;
		return null;
	}
	value &value::operator[](std::string const &enrty)
	{
		object::iterator p;
		object &members=object();
		return members[path];
	}

	value::value() : d(new value_data) { }
	valie::value(value const &other) : d(other.d) {}
	value const &operator=(value const &v) 
	{
		d=other.d;
	}
	~value()  {}

	
	namespace {
		std::string uchar(std::string::const_iterator p,std::string::const_iterator e)
		{
			// TODO
			return uchar(*p);	
		}
		std::string uchar(unsigned v)
		{
			char buf[8];
			snprintf(buf,sizeof(buf),"\\u%04X",v);
			std::string res=buf;
			return res;
		}
		std::string value::escape(std::string const &input,bool utf)
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

		void pad(std::ostream &out,int tb) const
		{
			for(;tb > 0;tb--) out<<'\t';
		}

		void indent(std::ostream &out,char c,int &tabs) const
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
		return d->type();
	}
	
	void value::write(std::ostream &out,int tabs,bool utf) const 
	{
		if(is_null()) {
			out<<"null";
			return;
		}
		switch(d->type_) {
		case is_number:
			out<<d->num_;
			break;
		case is_string:
			out<<escape((d->str_),utf);
			break;
		case is_boolean:
			out<< d->bool_ ? "true" : "false" ;
		case is_array:
			{
				array const &a=d->arr_;
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
			object const &obj=d->obj_;
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
			break;
		default:
			throw std::bad_cast();
		}
	}
	
	std::string value::save(int how=(utf8 | compact))
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

/*	namespace {
		struct tockenizer {
			value::input_stream &input_;
			tockenizer(value::input_stream  &input) : input_(input)
			{
			}
			std::string string_;
			double number_;
			bool boolean_;
			enum { 
				error = 0,
				null = 256,
				string = 257,
				boolean = 258 ,
				number = 259,
				eof = 260
			};
			bool read_number()
			{
				std::string num;
				num.reserve(32);
				c=input_.get();
				num+=c;
				if(c=='0') {
					number_=0;
					return true;
				}
				while((c=input_.get())!=eof && '0'<=c && c<='9')
					num+=c;
				if(c=='.') {
					num+=c;
					c=input_.get();
					if(c<'0' || '9'<c)
						return false;
					num+=c;
					while((c=input_.get())!=eof && '0'<=c && c<='9')
						num+=c;
				}
				if(c=='E' || c=='e')

			}
			int next()
			{
				int c=input.get();
				for(;;) {
					switch(c) {
					case -1:
						return eof;
					case '\t':
					case '\f':
					case '\r':
					case '\n':
					case ' ':
						break;
					case '{':
					case '[':
					case ']':
					case '}':
					case ':':
					case ',':
						return c; 
					case '/':
						c=input_.get();
						if(c=='/') {
							while((c=input_.get())!=-1 && c!='\n')
								;
							if(c==-1)
								return eof;
							if(c=='\n')
								break;
						}
						else if(c=='*') {
							while((c=input_.get())!=-1) {
								if(c=='*') {
									c=input_.get()
									if(c=='/') 
										break;
									if(c==-1)
										return error;
								}
							}
							break;
						}
						else
							return error;
						break;
					case 't':
						if(!is("rue"))
							return error;
						boolean_=true;
						return boolean;
					case 'f':
						if(!is("alse"))
							return error;
						boolean_=false;
						reurn boolean;
					case 'n':
						if(!is("ull"))
							return error;
						return null;
					case '"':
						input_.unget('"');
						if(!read_string())
							return error;
						return string;
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
						input_.unget(c);
						if(!read_number())
							return error;
						return number;
					default:
						return error;
					}
				}
			}
		}
	}


	value parse(input_stream &input,bool &ok)
	{
		value result;
		tockenizer tock(input);
		std::stack<int> stack;
		ok=true;
		int tocken;
		for(;;) {
			tocken=tock.get();
			if(tocken==tockenizer::error) {
				ok=false;
				return value();
			}
			if(tocken==tockenizer::eof)


	}*/


} // json

} // cppcms
