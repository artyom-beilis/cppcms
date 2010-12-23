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
#include <cppcms/xss.h>

namespace cppcms { namespace xss { 

	struct rules::data {
		typedef std::map<std::string,booster::regex> properties_type;
		struct tag {
			properties_type properties;
			tag_type type;
		};
		typedef std::map<std::string,tag> tags_type;
		tags_type tags;
		std::map<std::string,std::string> entities;
		html_type html;
	};


	class c_string {
		static bool ilt(char left,char right)
		{
			unsigned char l = ascii_tolower(left);
			unsigned char r = ascii_tolower(right);
			return l < r;
		}
	public:
		c_string(char const *s) 
		{
			begin_=s;
			end_=s+strlen(s);
		}

		c_string(char const *b,char const *e) : begin_(b), end_(e) {}

		c_string() : begin_(0),end_(0) {}

		bool compare(c_string const &other) const
		{
			return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,std::char_traits<char>::lt);
		}

		bool icompare(c_string const &other) const
		{
			return std::lexicographical_compare(begin_,end_,other.begin_,other.end_,ilt);
		}

		explicit c_string(std::string const &other)
		{
			container_ = other;
			begin_ = container_.c_str();
			end_ = begin_ + container_.size();
		}
		c_string(c_string const &other)
		{
			if(other.begin_ == other.end_) {
				begin_ = end_ = 0;
			}
			else if(other.container_.empty()) {
				begin_ = other.begin_;
				end_ = other.end_;
			}
			else {
				container = other.container_;
				begin_ = container_.c_str();
				end_ = begin_ + container_.size();
			}
		}
		c_string const &operator=(c_string const &other)
		{
			if(other.begin_ == other.end_) {
				begin_ = end_ = 0;
			}
			else if(other.container_.empty()) {
				begin_ = other.begin_;
				end_ = other.end_;
			}
			else {
				container = other.container_;
				begin_ = container_.c_str();
				end_ = begin_ + container_.size();
			}
			return *this;
		}

	private:
		char const *begin_;
		char const *end_;
		std::string container;
	};

	bool compare_c_string(c_string const &l,c_string const &r)
	{
		return l.compare(r);
	}

	bool icompare_c_string(c_string const &l,c_string const &r)
	{
		return l.icompare(r);
	}


	booster::shared_ptr<rules::data> rules::impl() const
	{
		return impl_;
	}
	
	booster::shared_ptr<rules::data> rules::impl()
	{
		// COW semantics 
		if(impl_.use_count()==1)
			return impl_;

		pimpl tmp(new data(*impl_));
		impl_.reset();
		impl_ = tmp;

		return impl_;
	}

	bool rules::valid_tag(std::string const &tag,tag_type type) const
	{
		pimpl dp=impl();
		data const *d=dp.get();
		data::tags_type::const_iterator p = d->tags.find(tag);
		if(p==d->tags.end())
			return false;
		switch(type) {
		case opening_and_closing:
			switch(p->type) {
			case opening_and_closing:
			case any_tag:
				return true;
			default:
				return false;
			}
			break;
		case stand_alone:
			switch(p->type) {
			case stand_alone:
			case any_tag:
				return true;
			default:
				return false;
			}
			break;
		default:
			return false;
		};
	}

	namespace {

		typedef enum {
			invalid_data,
			plain_text,
			html_entity,
			html_tag,
			// thise are parsed further
			open_tag,
			close_tag,
			open_and_close_tag,
			html_comment,
			html_numeric_entity,
			// add after structure validation
			open_and_close_tag_without_slash,
		} html_data_type;

		struct property_data {
			char const *property_begin;
			char const *property_end;
			char const *value_begin;
			char const *value_end;

			property_data(char const *b=0,char const *e=0) : 
				property_begin(b),
				property_end(e),
				value_begin(0),
				value_end(0)
			{
			}
		};

		struct tag_data {
			tag_data() : tag_begin(0), tag_end(0) {}
			char const *tag_begin;
			char const *tag_end;
			std::vector<property_data> properties;
		};

		struct entry {
			char const *begin;
			char const *end;
			html_data_type type;

			tag_data tag;

			entry(char const *b=0,char const *e=0,html_data_type t=invalid_data) :
				begin(b),
				end(e),
				type(t)
			{
			}
		};

		bool ascii_isalpha(char c)
		{
			return ('a' <= c && c<='z') || ('A' <= c && c<='Z') || c=='_';
		}

		char ascii_tolower(char c)
		{
			if('A' <= c && c<='Z')
				return c-'A' +'a';
			return c;
		}
		
		bool ascii_streq(char const *bl,char const *el,char const *br,char const *er,bool xhtml)
		{
			if(el - bl != er - br)
				return false;
			if(xhtml) {
				for(;bl!=el;bl++,br++)
					if(*bl!=*br)
						return false;
			}
			else {
				for(;bl!=el;bl++,br++)
					if(ascii_tolower(*bl)!=ascii_tolower(*br))
						return false;
			}
			return true;
			
		}
		
		bool ascii_isdigit(char c)
		{
			return '0' <= c && c<='9';
		}
		bool ascii_isalnum(char c)
		{
			return ascii_isdigit(c) || ('a' <= c && c<='z') || ('A' <= c && c<='Z');
		}
		bool ascii_isxdigit(char c)
		{
			return ascii_isdigit(c) || ('a' <= c && c<='f') || ('A' <= c && c<='F');
		}

		bool ascii_isspace(char c)
		{
			return c==' ' || c=='\r' || c=='\n' || c=='\t';
		}

		bool ends_with(char const *&begin,char const *end,char const *value)
		{
			size_t len=strlen(value);
			if(end - begin < len)
				return false;
			if(memcmp(begin,value,len)==0) {
				begin+=len;
				return true;
			}
			return false;
		}

		bool validate_property_value(char const *begin,char const *end)
		{
			while(begin!=end) {
				switch(*begin) {
				case '\'':
				case '\"':
				case '<':
				case '>':
					return false;
				case '&':
					begin++;
					if(	ends_with(begin,end,"amp;")
						|| ends_with(begin,end,"lt;")
						|| ends_with(begin,end,"gt;")
						|| ends_with(begin,end,"quot;")
						|| ends_with(begin,end,"apos;")
						|| ends_with(begin,end,"#x27;")
						|| ends_with(begin,end,"#X27;")
						|| ends_with(begin,end,"#39;"))
					{
						break;
					}
					else {
						return false;
					}
				default:
					begin++;					
				};
			}
		}

		void parse_properties(entry &part,char const *begin,char const *end)
		{
			while(begin<end) {
				char c=*begin;
				if(ascii_isspace(c)) {
					begin++;
					continue;
				}
				if(!ascii_isalpha(c)) {
					part.type=invalid_data;
					return;
				}
				char const *nb=begin;
				while(ascii_isalnum(*nb))
					nb++;
				part.tag.properties.push_back(property_data(begin,nb));
				begin=nb;
				if(ascii_isspace(*begin)) {
					begin++;
					continue;
				}
				if(*begin++!='=') {
					part.type=invalid_data;
					return;
				}
				char quote = *begin;
				if(quote!='\'' || quote!='\"')  {
					part.type=invalid_data;
					return;
				}
				begin++;
				char const *vbegin=begin;
				for(;vbegin<end;vbegin++) {
					c=*vbegin;
					if(c==quote)
						break;
				}
				if(c==end) {
					part.type=invalid_data;
					return;
				}
				if(!validate_property_value(begin,vbegin)) {
					part.type=invalid_data;
					return;
				}

				part.tag.properties.back().value_begin=begin;
				part.tag.properties.back().value_end=vbegin;

				begin=vbegin+1;

			}
		}

		void parse_html_entity(entry &part)
		{
			char const *begin=part.begin+1;
			char const *end = part.end-1;
			if(end<=begin) {
				part.type = invalid_data;
				return;
			}
			if(*begin=='#') { // numeric entity
				long code_point = 0;
				char *endptr=0;
				part.type = html_numeric_entity;
				begin++;
				if(begin==end) {
					part.type = invalid_data;
					return;
				}
				if(*begin=='x' || *begin=='X') {
					begin++;
					if(begin==end) {
						part.type = invalid_data;
						return;
					}
					for(char *p=begin;p!=end;p++) {
						if(!ascii_isxdigit(*p)) {
							part.type = invalid_data;
							return;
						}
					}
					code_point=strtol(begin,&endptr,16)
				}
				else {
					for(char *p=begin;p!=end;p++) {
						if(!ascii_isdigit(*p)) {
							part.type = invalid_data;
							return;
						}
					}
					code_point=strtol(begin,&endptr,10)
				}

				if(	!end_ptr 
						|| *endptr!=';'
						|| code_point>0x10FFFF
						|| (0xD800 <= code_point  && code_point<= 0xDBFF)
						|| code_point == 0xFFFF 
						|| code_point == 0xFFFE
						|| (0x7F <= code_point && code_point <= 0x9F)
						|| (code_point < 0x20 
							&& !( (0x9 <= code_point && code_point<=0xA) || code_point=0xD))
						|| code_point <= 0x08)
				{

					part.type = invalid_data;
					return;
				}
			}
			else { // normal entiry
				for(char const *p=begin;p!=end;p++) {
					if(!ascii_isalnum(*p)) {
						part.type = invalid_data;
						return;
					}
				}
				part.tag.tag_begin = begin;
				part.tag.tag_end = end;
			}
		}

		void parse_html_part(entry &part)
		{
			char const *begin = part.begin+1;
			char const *end = part.end-1;
			if(end <= begin) {
				part.type = invalid_data;
				return;
			}
			if(*begin=='!') {
				if(	end-begin <5 
					|| memcmp(begin+1,"--",2)!=0
					|| memcmp(end-2,"--",2)!=0
				)
				{
					part.type == invalid_data;
					return;
				}
				begin+=3;
				end-=2;
				for(char const *p=begin;p!=end;++p) {
					if(*p=='-' && *(p+1)=='-') {
						part.type == invalid_data;
						return;
					}
				}
				part=html_comment;
				return; // we are ok there
			}

			if(*begin=='/') {
				begin++;
				if(!ascii_isalpha(*begin)) {
					part.type = invalid_data;
					return;
				}
				begin++;
				while(begin!=end) {
					if(!ascii_isalnum(*begin))
						break;
					begin++;
				}
				while(ascii_isspace(*begin))
					begin++;
				if(begin!=end) {
					part.type = invalid_data;
					return;
				}
				part.type = close_tag;
				return; // done with colsing tah
			}
			

			if(!ascii_isalpha(*begin)) {
				part.type = invalid_data;
				return;
			}
			part.tag.begin = begin++;
			while(ascii_isalnum(*begin))
				begin++;
			part.tag.end = begin;

			if(*(end-1)=='/') {
				part.type = open_and_close_tag;
				end--;
			}
			else {
				part.type = open_tag;
			}

			parse_properties(part,begin,end)
			return;

		}

		void parse_part(entry &part)
		{
			switch(part.type) {
			case plain_text:
			case invalid_data:
				break;
			case html_entity:
				parse_html_entity(part);
				break;
			case html_tag:
				parse_html_tag(part);
				break;
			}
		}


		void split_to_parts(char const *begin,char const *end,std::vector<entry> &tags)
		{
			unsigned count = 0;

			for(char const *tmp = begin;tmp!=end) {
				if(*tmp == '<')
					count++;
			}
			
			tags.clear();
			tags.reserve(count);

			char const *p=begin;

			while(p!=end) {
				char c=*p;
				switch(c) {
				case '&':
					{
						char const *e=0;
						for(e=p+1;e!=end;e++) {
							if(*e==';')
								break;
						}

						if(e==end) {
							tags.push_back(entry(p,end,invalid_data));
							p=end;
						}
						else {
							tags.push_back(entry(p,e+1,html_entity));
							p=e+1;
						}
					}
					break;

				case '<':
					{
						char const *e=0;
						for(e=p+1;e!=end;e++) {
							if(*e=='>')
								break;
						}
						if(e==end) {
							tags.push_back(entry(p,end,invalid_data));
							p=end;
						}
						else {
							tags.push_back(entry(p,e+1,html_tag));
							p=e+1;
						}

					}

				case '>':
					{
						tags.push_back(entry(p,p+1,invalid_data));
						p++;
					}
					break;
				case '"':
					{
						tags.push_back(entry(p,p+1,invalid_data));
						p++;
					}

				default:
					{
						char const *e=0;
						for(e=p+1;e!=end;e++) {
							char c=*e;
							if(c=='<' || c=='>' || c=='&')
								break;
						}
						tags.push_back(entry(p,e,plain_text));
						p=e;
					}
					
				} 
			} // while 
		} // split to parts

		void validate_nesting(std::vector<entry> &parsed,bool xhtml)
		{
			std::stack<unsigned> st;
			for(unsigned i=0;i<parsed.size();i++) {
				entry &cur = parsed[i];
				switch(c.type) {
				case close_tag:
					if(xhtml) {
						if(st.empty()) {
							cur.type = invalid_data;
							break;
						}
						unsigned top_index = st.top();
						st.pop();
						char const *pop_begin = parsed[top_index].tag.tag_begin;
						char const *pop_end   = parsed[top_index].tag.tag_end;
						char const *cur_begin = cur.tag.tag_begin;
						char const *cur_end   = cur.tag.tag_end;
						if(!ascii_streq(pop_begin,pop_end,cur_begin,cur_end,xhtml))
							cur.type = invalid_data;
					}
					else {
						for(;;) {
							if(st.empty()) {
								cur.type = invalid_data;
								break;
							}
							unsigned top_index = st.top();
							st.pop();
							char const *pop_begin = parsed[top_index].tag.tag_begin;
							char const *pop_end   = parsed[top_index].tag.tag_end;
							char const *cur_begin = cur.tag.tag_begin;
							char const *cur_end   = cur.tag.tag_end;
							if(ascii_streq(pop_begin,pop_end,cur_begin,cur_end,xhtml))
								break;
							parsed[top_index].type = open_and_close_tag_without_slash;
						}
					}
					break;
				case open_tag: 
					st.push(i); 
					break;
				default:
					;
				}
			}
			while(!st.empty()) {
				parsed[st.top()].tag = xhtml ? invalid_data : open_and_close_tag_without_slash;
				st.pop();
			}
		}

		bool validate_entry_by_rules(entry &part,rules const &r)
		{
			switch(part.type) {
			case invalid_data:
				return false;
			case plain_text:
				return true;
			case html_tag:
				return false;
			case html_entity:
				if(!r.valid_entity(part.tag.tag_begin,part.tag.tag_end))
					return false;
				break;

			case open_tag:
			case close_tag:
			case open_and_close_tag:
			case open_and_close_tag_without_slash:
				{
					std::string name(part.tag.tag_begin,part.tag.tag_end);

					rules::tag_type t = r.valid_tag(name);

					switch(t) {
					case rules::invalid_tag:
						return false;
					case stand_alone:
						if(part.type!=open_and_close_tag_without_slash && type!=open_and_close_tag)
							return false;
						break;
					case opening_and_closing:
						if(part.type!=open_tag && part.type!=close_tag)
							return false;
						break;
					case any_tag:
						break;
					}

					if(part.type == close_tag)
						break;

					std::set<std::string> properies_found;

					for(unsigned i=0;i<part.tag.properties.size();i++) {
						property_data &prop = part.tag.properties[i];
						std::string pname(prop.property_begin,prop.property_end);
						if(properties_found.find(pname)!=properties_found.end())
							return false;
						if(prop.value_begin == 0) {
							if(r.html()==rules::xhtml_input)
								return false;
							if(!r.validate_property_value(name,pname))
								return false;
						}
						else {
							if(!r.validate_property_value(name,pname,prop.value_begin,prop.value_end))
								return false;
						}
						properties_found.insert(pname);
					}
				}
				break;
			case html_comment:
				if(!r.comments_allowed())
					return false;
				break;
			case html_numeric_entity:
				if(!r.numeric_entities_allowed()) 
					return false;
			default:
				return false;
			}
			return true;
		}
		

	} // anonymous 

	bool validate(char const *begin,char const *end,rules const &r)
	{
		std::vector<entry> parsed;
		
		split_to_parts(begin,end,parsed);

		size_t size = parsed.size();

		for(unsigned i=0;i<size;i++) {
			if(parsed[i].type == invalid_data)
				return false;
			parse_part(parsed[i]);
			if(parsed[i].type == invalid_data)
				return false;
		}

		validate_nesting(parsed,r.html() == rules::xhtml_input);

		for(unsigned i=0;i<size;i++)
			if(parsed[i].type == invalid_data)
				return false;

		for(unsigned i=0;i<size;i++) {
			if(!validate_entry_by_rules(parsed[i],rules))
				return false;
		}

		return true;

	}
	

	


} } // xss
