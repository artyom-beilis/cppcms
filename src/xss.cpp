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
#include <cppcms/encoding.h>
#include <booster/locale/encoding.h>
#include <booster/regex.h>
#include <stack>
#include <map>
#include <set>
#include <stdlib.h>
#include <string.h>

#include <iostream>

namespace cppcms { namespace xss { 

	using namespace details;
	
	struct compare_c_string {
		bool operator()(details::c_string const &l,details::c_string const &r) const
		{
			return l.compare(r);
		}
	};

	struct icompare_c_string {
		bool operator()(details::c_string const &l,details::c_string const &r) const
		{
			return l.icompare(r);
		}
	};

	struct regex_functor {
	public:
		regex_functor(booster::regex const &r=booster::regex()) :
			r_(r)
		{
		}
		bool operator()(char const *b,char const *e) const
		{
			return booster::regex_match(b,e,r_);
		}
	private:
		booster::regex r_;
	};

	bool integer_property_functor(char const *begin,char const *end) 
	{
		if(begin!=end && *begin=='-')
			begin++;
		if(begin==end)
			return false;
		while(begin!=end) {
			char c= *begin++;
			if(c<'0' || '9'<c)
				return false;
		}
		return true;
	}

	
	struct basic_rules_holder {
		
		typedef std::set<details::c_string,compare_c_string> entities_type;
		typedef rules::validator_type validator_type;
		entities_type entities;
		
		bool valid_entity(c_string const &name) const
		{
			return entities.find(name)!=entities.end();
		}
		void add_entity(std::string const &name)
		{
			entities.insert(c_string(name));
		}
		virtual void add_tag(std::string const &name,rules::tag_type type) = 0;
		virtual void add_property(std::string const &tname,std::string const &pname,validator_type const &r) = 0;
		virtual rules::tag_type valid_tag(c_string const &t) const = 0;
		virtual bool valid_boolean_property(c_string const &tname,c_string const &pname) const = 0;
		virtual bool valid_property(c_string const &tname,c_string const &pname,c_string const &value) const = 0;
		virtual ~basic_rules_holder(){}
	};
	
	template<typename Comp,bool IsXHTML>
	struct rules_holder: public basic_rules_holder {
		
		rules_holder()
		{
			add_entity("lt");
			add_entity("gt");
			add_entity("amp");
			add_entity("quot");
		}
		
		typedef std::map<details::c_string,validator_type,Comp> properties_type;
		struct tag {
			properties_type properties;
			rules::tag_type type;
		};
		typedef std::map<details::c_string,tag,Comp> tags_type;
		tags_type tags;
		
		void add_tag(std::string const &name,rules::tag_type type)
		{
			tags[c_string(name)].type=type;
		}
		void add_property(std::string const &tname,std::string const &pname,validator_type const &r)
		{
			c_string cname(tname);
			if(tags.find(cname)==tags.end())
				tags[cname].type=rules::invalid_tag;
			
			tags[c_string(tname)].properties[c_string(pname)]=r;
		}
		rules::tag_type valid_tag(c_string const &t) const
		{
			typename tags_type::const_iterator p = tags.find(t);
			if(p==tags.end())
				return rules::invalid_tag;
			return p->second.type;
		}
		
		bool valid_boolean_property(c_string const &tname,c_string const &pname) const
		{
			if(IsXHTML) 
				return false;
			typename tags_type::const_iterator pt = tags.find(tname);
			if(pt==tags.end())
				return false;
			typename properties_type::const_iterator pp = pt->second.properties.find(pname);
			if(pp == pt->second.properties.end())
				return false;
			if(pp->second.empty())
				return true;
			return false;
		}
		
		bool valid_property(c_string const &tname,c_string const &pname,c_string const &value) const
		{
			typename tags_type::const_iterator pt = tags.find(tname);
			if(pt==tags.end())
				return false;
			typename properties_type::const_iterator pp = pt->second.properties.find(pname);
			if(pp == pt->second.properties.end())
				return false;
			if(pp->second.empty()) {
				if(IsXHTML) {
					Comp cmp;
					if(!cmp(pname,value) && !cmp(value,pname))
						return true;
					return false;
				}
				else {
					return false; // Should be boolean in HTML
				}
			}
			if(pp->second(value.begin(),value.end()))
				return true;
			return false;
		}
		
		
	};
	
	struct rules::data {
		
		rules_holder<compare_c_string,true> xhtml_rules;
		rules_holder<icompare_c_string,false> html_rules;
		
		bool is_xhtml;
		bool comments_allowed;
		bool numeric_entities_allowed;
		std::string encoding;
		
		data() :
			is_xhtml(true),
			comments_allowed(false),
			numeric_entities_allowed(false)
		{
		}
	};

	basic_rules_holder &rules::impl() 
	{
		if(d->is_xhtml)
			return d->xhtml_rules;
		else
			return d->html_rules;
	}
	basic_rules_holder const &rules::impl()  const
	{
		if(d->is_xhtml)
			return d->xhtml_rules;
		else
			return d->html_rules;
	}
	
	rules::rules() : d(new data())
	{
	}
	rules::rules(rules const &other) : d(other.d)
	{
	}
	rules const &rules::operator=(rules const &other)
	{
		d=other.d;
		return *this;
	}
	rules::~rules()
	{
	}
	
	rules::html_type rules::html() const
	{
		if(d->is_xhtml)
			return xhtml_input;
		else
			return html_input;
	}
	
	void rules::html(rules::html_type t)
	{
		switch(t) {
		case html_input:
			d->is_xhtml=false;
			break;
		case xhtml_input:
			d->is_xhtml=true;
			break;
		}
	}

	void rules::encoding(std::string const &enc)
	{
		d->encoding = enc;
	}
	std::string rules::encoding() const
	{
		return d->encoding;
	}
	
	void rules::add_tag(std::string const &tag_name,rules::tag_type t)
	{
		impl().add_tag(tag_name,t);
	}
	
	void rules::add_entity(std::string const &entity)
	{
		impl().add_entity(entity);
	}
	
	bool rules::numeric_entities_allowed() const
	{
		return d->numeric_entities_allowed;
	}
	void rules::numeric_entities_allowed(bool v)
	{
		d->numeric_entities_allowed=v;
	}
	
	void rules::add_boolean_property(std::string const &tag_name,std::string const &property)
	{
		add_property(tag_name,property,validator_type());
	}
	void rules::add_property(std::string const &tag_name,std::string const &property,booster::regex const &r)
	{
		impl().add_property(tag_name,property,regex_functor(r));
	}
	void rules::add_property(std::string const &tag_name,std::string const &property,rules::validator_type const &v)
	{
		impl().add_property(tag_name,property,v);
	}
	void rules::add_integer_property(std::string const &tag_name,std::string const &property)
	{
		add_property(tag_name,property,integer_property_functor);
	}
	
	bool rules::comments_allowed() const
	{
		return d->comments_allowed;
	}
	
	void rules::comments_allowed(bool v)
	{
		d->comments_allowed=v;
	}
	
	rules::tag_type rules::valid_tag(c_string const &t) const
	{
		return impl().valid_tag(t);
	}
	
	bool rules::valid_boolean_property(details::c_string const &tag,details::c_string const &property) const
	{
		return impl().valid_boolean_property(tag,property);
	}
	
	bool rules::valid_property(details::c_string const &tag,details::c_string const &property,details::c_string const &value) const
	{
		return impl().valid_property(tag,property,value);
	}

	bool rules::valid_entity(details::c_string const &val) const
	{
		return impl().valid_entity(val);
	}


	booster::regex rules::uri_matcher()
	{
		return uri_matcher("(http|https|ftp|mailto|news|nntp)");
	}

	booster::regex rules::uri_matcher(std::string const &scheme)
	{
		std::string sub_delims="(['!,;=\\$\\(\\)\\*\\+]|&amp;|&apos;)";
		std::string gen_delims="[\\:\\/\\?\\#\\[\\]\\@]";
		std::string reserverd="(" + gen_delims + "|" + sub_delims + ")";
		std::string unreserved="[a-zA-Z_0-9\\-\\.~]";
		std::string pct_encoded="%[0-9a-fA-F][0-9a-fA-F]";
		std::string pchar="(" + unreserved + "|" + pct_encoded + "|" + sub_delims + "|:|\\@)";
		std::string query="(" + pchar + "|/|\\?)*";
		std::string fragment="(" + pchar + "|/|\\?)*";
		std::string segment="(" + pchar + ")*";
		std::string segment_nz="(" + pchar + ")+";
		std::string segment_nz_nc="(" + unreserved + "|" + pct_encoded + "|" + sub_delims + "|" + "\\@)+";
		std::string path_rootless = "(" + segment_nz + "(/" + segment + ")*)";
		std::string path_noscheme = "(" + segment_nz_nc + "(/"+ segment +")*)";
		std::string path_absolute = "/("+ segment_nz + "(/"+ segment +")*)?";
		std::string path_abempty  = "(/" + segment+")*";
		std::string path="(" + path_abempty + "|" + path_absolute + "|" + path_noscheme + "|" + path_rootless +")?";
		std::string reg_name = "(" + unreserved + "|" + pct_encoded + "|" + sub_delims + ")*";
		std::string dec_octet = "([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])";
		std::string ipv4addr = "(" + dec_octet + "\\." + dec_octet + "\\." + dec_octet + "\\." + dec_octet +")";
		std::string port = "([0-9]*)";
		std::string host = "(" + ipv4addr + "|" + reg_name + ")";
		std::string userinfo= "(" + unreserved + "|" + pct_encoded + "|" + sub_delims +"|\\:)*";
		std::string authority = "((" + userinfo +"\\@)?" + host + "(\\:"+port+")?)";
		std::string relative_part = "(//" + authority + path_abempty 
						+"|"+path_absolute
						+"|"+path_noscheme
						+")?";
		std::string relative_ref = "(" + relative_part + "(\\?" + query + ")?(#" + fragment +")?)";


		std::string hier_part = "(//" + authority + path_abempty 
						+"|"+path_absolute
						+"|"+path_rootless
						+")?";

		std::string uri = "(" + scheme +":" +  hier_part + "(\\?" + query + ")?(#" + fragment +")?)";

		std::string uri_reference = "(" + uri + "|" + relative_ref +")";

#ifdef DEBUG_XSS_URI_REGEXS

		std::string strings[] = {
			sub_delims,
			gen_delims,
			reserverd,
			unreserved,
			pct_encoded,
			pchar,
			query,
			fragment,
			segment,
			segment_nz,
			segment_nz_nc,
			path_rootless,
			path_noscheme,
			path_absolute,
			path_abempty,
			path,
			reg_name,
			dec_octet,
			ipv4addr,
			port,
			host,
			userinfo,
			authority,
			relative_part,
			relative_ref,
			hier_part,
			uri,
			uri_reference
		};

		for(unsigned i=0;i<sizeof(strings)/(sizeof(strings[0]));i++) {
			std::cout << i << " " << strings[i] << std::endl;
			booster::regex r(strings[i]);
		}
#endif // DEBUG_XSS_REGEXS

		return booster::regex(uri_reference);
	}

	// second implementation

	class uri_parser {

		struct save_point;
		friend struct save_point;
		char const *scheme_start_;
		char const *scheme_end_;
		char const *begin_;
		char const *end_;
		bool is_relative_;

		struct save_point {
			save_point(uri_parser *self) : 
				self_(self) ,
				sp_(self_->begin_)
			{
			}
			void commit()
			{
				sp_ = self_->begin_;
			}
			~save_point()
			{
				self_->begin_ = sp_;
			}
			uri_parser *self_;
			char const *sp_;
		};



		bool follows(char c)
		{
			if(begin_ != end_ && *begin_ == c){
				begin_++;
				return true;
			}
			return false;
		}

		bool follows(char const *s)
		{
			int n = strlen(s);
			if(end_ - begin_ >= n && memcmp(begin_,s,n) == 0) {
				begin_ +=n;
				return true;
			}
			return false;
		}

		// RFC 3305

		//	sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
		//	/ "*" / "+" / "," / ";" / "="

		bool sub_delims()
		{
			if(begin_ == end_)
				return false;
			if(follows("&amp;") || follows("&apos;"))
				return true;
			switch(*begin_) {
			case '!' : case '$' : case '(' :
			case ')' : case '*' : case '+' : 
			case ',' : case ';' : case '=' :
			case '\'':
				begin_ ++;
				return true;
			}
			return false;
		}

		//	gen-delims    = ":" / "/" / "?" / "#" / "[" / "]" / "@"
		bool gen_delims()
		{
			if(begin_ == end_)
				return false;
			switch(*begin_) {
			case ':' : case '/' : case '?' : case '#' :
			case '[' : case ']' : case '@' :
				begin_ ++;
				return true;
			}
			return false;
		}

		//	reserved      = gen-delims / sub-delims
		bool reserverd()
		{
			return gen_delims() || sub_delims();
		}
		static bool is_digit(char c)
		{
			return '0'<=c && c<='9';
		}
		static bool is_alapha(char c)
		{
			return ('a'<=c && c<='z') || ('A'<=c && c<='Z');
		}

		//	unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
		bool unreserved()
		{
			if(begin_ == end_)
				return false;
			char c=*begin_;
			if(	is_alapha(c) || is_digit(c) 
				|| c=='-' || c=='.' || c=='_' || c=='~')
			{
				begin_++;
				return true;
			}
			return false;
		}

		static bool is_hex(char c)
		{
			return is_digit(c) || ('a'<=c && c<='f') || ('A'<=c && c<='F');
		}

		//	pct-encoded   = "%" HEXDIG HEXDIG
		bool pct_encoded()
		{
			if(end_ - begin_ >=3 && begin_[0]=='%' && is_hex(begin_[1]) && is_hex(begin_[2])) {
				begin_+=3;
				return true;
			}
			return false;
		}

		//	pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
		bool pchar()
		{
			return unreserved() || pct_encoded() || sub_delims() || follows(':') || follows('@');
		}

		//	query         = *( pchar / "/" / "?" )
		//	fragment      = *( pchar / "/" / "?" )

		bool query()
		{
			while(pchar() || follows('/') || follows('?'))
				;
			return true;
		}
		bool fragment()
		{
			return query();
		}
		//	segment       = *pchar

		bool segment()
		{
			while(pchar())
				;
			return true;
		}

		//	segment-nz    = 1*pchar
		//	; non-zero-length segment without any colon ":"
		bool segment_nz()
		{
			if(!pchar())
				return false;
			while(pchar())
				;
			return true;
		}
		//	segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
		bool segment_nz_nc()
		{
			int count = 0;
			while(unreserved() || pct_encoded() || sub_delims() || follows('@'))
				count++;
			return count != 0;
		}
		//	path-rootless = segment-nz *( "/" segment )
		bool path_rootless()
		{
			if(!segment_nz())
				return false;
			save_point sp(this);
			while(follows('/') && segment())
				sp.commit();
			return true;
		}

		//	path-noscheme = segment-nz-nc *( "/" segment )
		bool path_noscheme()
		{
			if(!segment_nz_nc())
				return false;
			save_point sp(this);
			while(follows('/') && segment())
				sp.commit();
			return true;
		}
		//	path-absolute = "/" [ segment-nz *( "/" segment ) ]
		bool path_absolute()
		{
			if(!follows('/'))
				return false;
			save_point sp(this);
			if(segment_nz()) {
				sp.commit();
				while(follows('/') && segment())
					sp.commit();
			}
			return true;
		}
		//	path-abempty  = *( "/" segment )
		bool path_abempty()
		{
			save_point sp(this);
			while(follows('/') && segment())
				sp.commit();
			return true;
		}
		//	path          = path-abempty    ; begins with "/" or is empty
		//	/ path-absolute   ; begins with "/" but not "//"
		//	/ path-noscheme   ; begins with a non-colon segment
		//	/ path-rootless   ; begins with a segment
		//	/ path-empty      ; zero characters

		bool path()
		{
			return path_absolute() || path_noscheme() || path_rootless() || path_abempty() || true;
		}

		// reg-name      = *( unreserved / pct-encoded / sub-delims )
		bool reg_name()
		{
			while(unreserved() || pct_encoded() || sub_delims())
				;
			return true;
		}
		// dec-octet     = DIGIT                 ; 0-9
		// / %x31-39 DIGIT         ; 10-99
		// / "1" 2DIGIT            ; 100-199
		// / "2" %x30-34 DIGIT     ; 200-249
		// / "25" %x30-35          ; 250-255
		bool dec_octet()
		{
			save_point sp(this);
			int count =0;
			int value = 0;
			char c;
			while(begin_!=end_ && is_digit((c=*begin_)) && count<3) {
				count ++;
				value = value * 10 + c-'0';
			}
			if(count == 0) 
				return false;
			if(value <= 9 && count !=1)
				return false;
			if(value <= 99 && count !=2)
				return false;
			if(value >255)
				return false;
			sp.commit();
			return true;
		}

		// IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet
		bool ipv4addr()
		{
			save_point sp(this);
			bool r= dec_octet() 
				&& follows('.')
				&& dec_octet()
				&& follows('.')
				&& dec_octet()
				&& follows('.')
				&& dec_octet();
			if(r) {
				sp.commit();
				return true;
			}
			return false;
		}

		// port          = *DIGIT
		bool port()
		{
			while((begin_!=end_) && is_digit(*begin_))
				begin_++;
			return true;
		}

		// host          = IP-literal / IPv4address / reg-name
		bool host()
		{
			return ipv4addr() || reg_name(); // TODO fix ipv6
		}

		// userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
		bool userinfo()
		{
			while(unreserved() || pct_encoded() || sub_delims() || follows(':'))
				;
			return true;
		}

		/// authority     = [ userinfo "@" ] host [ ":" port ]
		bool authority()
		{
			{
				save_point sp(this);
				if(userinfo() && follows('@') && host()) {
					sp.commit();
				}
				else if(host()) {
					sp.commit();
				}
				else {
					return false;
				}
			}
			{
				save_point sp(this);
				if(follows(':')  &&  port()) {
					sp.commit();
					return true;
				}
			}
			return true;
				
		}

		// relative-part = "//" authority path-abempty
		// 	/ path-absolute
		// 	/ path-noscheme
		// 	path-empty

		bool relative_part()
		{
			
			{
				save_point sp(this);
				if(authority() && path_abempty()) {
					sp.commit();
					return true;
				}
			}
			return path_absolute()  || path_noscheme() || true;
		}

		// relative-ref  = relative-part [ "?" query ] [ "#" fragment ]

		bool relative_ref()
		{
			if(!relative_part())
				return false;

			{
				save_point sp2(this);
				if(follows('?') && query())
					sp2.commit();
			}

			{
				save_point sp2(this);
				if(follows('#') && fragment())
					sp2.commit();
			}
			return true;
		}

		// hier-part     = "//" authority path-abempty
		//	/ path-absolute
		//	/ path-rootless
		//	/ path-empty


		bool hier_part()
		{
			{
				save_point sp(this);
				if(follows("//")) {
					if(!authority() || !path_abempty()) 
						return false;
					sp.commit();
					return true;
				}
			}

			return path_absolute() || path_rootless() || true;
		}

		// scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
		bool scheme()
		{
			if(begin_==end_ || !is_alapha(*begin_))
				return false;
			scheme_start_ = begin_;
			begin_++;
			char c;
			while(begin_!=end_ && (is_alapha((c=*begin_)) || is_digit(c) || c=='+' || c=='-' || c=='.'))
				begin_++;
			scheme_end_ = begin_;
			return true;
		}

		//URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
		bool uri()
		{
			save_point sp(this);
			
			if(!scheme() || !follows(':') || ! hier_part()){
				return false;
			}
			{
				save_point sp2(this);
				if(follows('?') && query())
					sp2.commit();
			}

			{
				save_point sp2(this);
				if(follows('#') && fragment())
					sp2.commit();
			}

			sp.commit();
			return true;

		}


		// URI-reference = URI / relative-ref
		bool uri_reference()
		{
			return uri() || (is_relative_ = relative_ref())!=false;
		}
	public:
		uri_parser(char const *begin,char const *end) :
			scheme_start_(0),
			scheme_end_(0),
			begin_(begin),
			end_(end),
			is_relative_(false)
		{
		}
		bool parse()
		{
			is_relative_ = false;
			return uri_reference() && begin_ == end_;
		}
		bool parse_relative()
		{
			is_relative_ = true;
			return relative_ref() && begin_ == end_;
		}

		bool parse_full()
		{
			is_relative_ = false;
			return uri_reference() && begin_ == end_;
		}

		bool has_scheme() const
		{
			return !is_relative_;
		}
		bool is_relative() const
		{
			return is_relative_;
		}
		char const *scheme_begin() const
		{
			return scheme_start_;
		}

		char const *scheme_end() const
		{
			return scheme_end_;
		}

	}; // uri_parser


	void rules::add_uri_property(std::string const &tag_name,std::string const &property)
	{
		add_property(tag_name,property,uri_validator());
	}

	void rules::add_uri_property(std::string const &tag_name,std::string const &property,std::string const &schema)
	{
		add_property(tag_name,property,uri_validator(schema));
	}
	
	struct uri_validator_functor {
	public:
		enum uri_type { both, relative, full };
		uri_validator_functor(uri_type type,booster::regex const &scheme = booster::regex()) :
			type_(type),
			scheme_(scheme)
		{
		}
		bool operator()(char const *begin,char const *end) const
		{
			//std::cout << ">>>>>>>>>>>>>>> " << std::string(begin,end) << std::endl;
			uri_parser parser(begin,end);
			switch(type_) {
			case both:
				if(!parser.parse())
					return false;
				if(parser.has_scheme()) {
					std::string scheme(parser.scheme_begin(),parser.scheme_end());
					//std::cout << "Scheme=" << scheme << " " << scheme_.str()<< std::endl;
					return booster::regex_match(parser.scheme_begin(),parser.scheme_end(),scheme_);
				}
				return true;
			case relative:
				return parser.parse_relative();
			case full:
				if(!parser.parse_full())
					return false;
				return booster::regex_match(parser.scheme_begin(),parser.scheme_end(),scheme_);
			};
			return false;
		}
	private:
		uri_type type_;
		booster::regex scheme_;
	};

	rules::validator_type rules::uri_validator()
	{
		return uri_validator("^(http|https|ftp|mailto|news|nntp)$");
	}
	rules::validator_type rules::uri_validator(std::string const &scheme,bool absolute_only )
	{
		return uri_validator_functor(
				absolute_only ? uri_validator_functor::full : uri_validator_functor::both,
				booster::regex(scheme));
	}
	rules::validator_type rules::relative_uri_validator()
	{
		return uri_validator_functor(uri_validator_functor::relative);
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
			tag_data() : 
				tag_begin(0),
				tag_end(0),
				pair(-1) 
			{
			}
			char const *tag_begin;
			char const *tag_end;
			int pair;
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
			if(begin >= end || size_t(end - begin) < len)
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
				char c=*begin;
				switch(c) {
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
			return true;
		}

		void parse_properties(entry &part,char const *begin,char const *end)
		{
			bool space_found = true;
			while(begin<end) {
				char c=*begin;
				if(ascii_isspace(c)) {
					begin++;
					space_found=true;
					continue;
				}
				else if(!space_found) {
					part.type=invalid_data;
					return;
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
				if(quote!='\'' && quote!='\"')  {
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
				if(vbegin==end) {
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
				space_found = false;

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
					for(char const *p=begin;p!=end;p++) {
						if(!ascii_isxdigit(*p)) {
							part.type = invalid_data;
							return;
						}
					}
					code_point=strtol(begin,&endptr,16);
				}
				else {
					for(char const *p=begin;p!=end;p++) {
						if(!ascii_isdigit(*p)) {
							part.type = invalid_data;
							return;
						}
					}
					code_point=strtol(begin,&endptr,10);
				}


				if(	!endptr 
					|| *endptr!=';'
					|| code_point>0x10FFFF
					|| (0xD800 <= code_point  && code_point<= 0xDBFF)
					|| code_point == 0xFFFF 
					|| code_point == 0xFFFE
					|| (0x7F <= code_point && code_point <= 0x9F)
					|| (code_point < 0x20 
						&& !( (0x9 <= code_point && code_point<=0xA) || code_point==0xD))
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

		void parse_html_tag(entry &part)
		{
			char const *begin = part.begin+1;
			char const *end = part.end-1;
			if(end <= begin) {
				part.type = invalid_data;
				return;
			}
			if(*begin=='/') {
				begin++;
				if(!ascii_isalpha(*begin)) {
					part.type = invalid_data;
					return;
				}
				char const *rbegin = begin;
				begin++;
				while(begin!=end) {
					if(!ascii_isalnum(*begin))
						break;
					begin++;
				}
				char const *rend=begin;
				while(ascii_isspace(*begin))
					begin++;
				if(begin!=end) {
					part.type = invalid_data;
					return;
				}
				part.tag.tag_begin=rbegin;
				part.tag.tag_end=rend;
				part.type = close_tag;
				return; // done with colsing tah
			}
			

			if(!ascii_isalpha(*begin)) {
				part.type = invalid_data;
				return;
			}
			part.tag.tag_begin = begin++;
			while(ascii_isalnum(*begin))
				begin++;
			part.tag.tag_end = begin;

			if(*(end-1)=='/') {
				part.type = open_and_close_tag;
				end--;
			}
			else {
				part.type = open_tag;
			}

			parse_properties(part,begin,end);
			return;

		}

		void parse_part(entry &part)
		{
			switch(part.type) {
			case html_entity:
				parse_html_entity(part);
				break;
			case html_tag:
				parse_html_tag(part);
				break;
			default:
				/* Nothing to do in other cases */ ;
			}
		}


		void split_to_parts(char const *begin,char const *end,std::vector<entry> &tags)
		{
			unsigned count = 0;

			for(char const *tmp = begin;tmp!=end;tmp++) {
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

					if(p+4 < end && p[1]=='!' && p[2]=='-' && p[3]=='-') {
						char const *e =p + 4;
						while(e<end-1) {
							if(e[0]=='-' && e[1]=='-') 
								break;
							e++;
						}
						if(e+2<end && e[2]=='>') {
							
							html_data_type type = html_comment;
							
							for(char const *tmp = p+4;tmp<e;tmp++) {
								char c=*tmp;
								///
								/// Prevent IE conditionals
								///
								if(c=='>' || c=='<' || c=='&') {
									type = invalid_data;
									break;
								}
							}

							tags.push_back(entry(p,e+3,type));
							p=e+3;
						}
						else {
							tags.push_back(entry(p,end,invalid_data));
							p=end;
						}

					}
					else {
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

					break;

				case '>':
					{
						tags.push_back(entry(p,p+1,invalid_data));
						p++;
					}
					break;
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
				switch(cur.type) {
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
						if(ascii_streq(pop_begin,pop_end,cur_begin,cur_end,xhtml)) {
							cur.tag.pair = top_index;
							parsed[top_index].tag.pair = i;
						}
						else {
							cur.type = invalid_data;
							parsed[top_index].type = invalid_data;
						}
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
							if(ascii_streq(pop_begin,pop_end,cur_begin,cur_end,xhtml)) {
								cur.tag.pair = top_index;
								parsed[top_index].tag.pair = i;
								break;
							}
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
				parsed[st.top()].type = xhtml ? invalid_data : open_and_close_tag_without_slash;
				st.pop();
			}
		}

		bool validate_entry_by_rules(entry &part,rules const &r)
		{
			using details::c_string;
			
			switch(part.type) {
			case invalid_data:
				return false;
			case plain_text:
				return true;
			case html_tag:
				return false;
			case html_entity:
				if(!r.valid_entity(c_string(part.tag.tag_begin,part.tag.tag_end)))
					return false;
				break;
			case html_comment:
				if(!r.comments_allowed())
					return false;
				break;
			case html_numeric_entity:
				if(!r.numeric_entities_allowed()) 
					return false;
				break;
			case open_tag:
			case close_tag:
			case open_and_close_tag:
			case open_and_close_tag_without_slash:
				{
					c_string name(part.tag.tag_begin,part.tag.tag_end);
					rules::tag_type t = r.valid_tag(name);

					switch(t) {
					case rules::invalid_tag:
						return false;
					case rules::stand_alone:
						if(part.type!=open_and_close_tag_without_slash && part.type!=open_and_close_tag)
							return false;
						break;
					case rules::opening_and_closing:
						if(part.type!=open_tag && part.type!=close_tag)
							return false;
						break;
					case rules::any_tag:
						break;
					}

					if(part.type == close_tag)
						break;
					
					bool xhtml = r.html()==rules::xhtml_input;
 	
					std::set<c_string,compare_c_string> xhtml_properties_found;
					std::set<c_string,icompare_c_string> html_properties_found;

					for(unsigned i=0;i<part.tag.properties.size();i++) {
						property_data &prop = part.tag.properties[i];
						c_string pname(prop.property_begin,prop.property_end);
						if(xhtml) {
							if(xhtml_properties_found.find(pname)!=xhtml_properties_found.end())
								return false;
							xhtml_properties_found.insert(pname);
						}
						else {
							if(html_properties_found.find(pname)!=html_properties_found.end())
								return false;
							html_properties_found.insert(pname);
						}
						if(prop.value_begin == 0) {
							if(!r.valid_boolean_property(name,pname))
								return false;
						}
						else {
							if(!r.valid_property(name,pname,c_string(prop.value_begin,prop.value_end)))
								return false;
						}
					}
				}
				break;
			default:
				return false;
			}
			return true;
		}
		

	} // anonymous 

	bool validate(char const *begin,char const *end,rules const &r)
	{
		std::string enc = r.encoding();
		size_t dummy_count = 0;
		
		std::string temp_input;

		if(!enc.empty()) {
			if(encoding::is_ascii_compatible(enc)) {
				if(!encoding::valid(enc,begin,end,dummy_count))
					return false;
			}
			else {
				try {
					std::string tmp = 
						booster::locale::conv::to_utf<char>(
							begin,end,enc,booster::locale::conv::stop);
					temp_input.swap(tmp);
					begin = temp_input.c_str();
					end = begin + temp_input.size();
				}
				catch(...) {
					return false;
				}
				if(!encoding::valid("UTF-8",begin,end,dummy_count))
					return false;
			}
		}
		
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
			if(!validate_entry_by_rules(parsed[i],r))
				return false;
		}

		return true;

	}
	
	bool validate_and_filter_if_invalid(	char const *begin,
						char const *end,
						rules const &r,
						std::string &output,
						filtering_method_type method,
						char repl_ch)
	{
		bool valid = true;
		
		std::string real_encoding = r.encoding();
		std::string work_encoding = real_encoding;

		std::string work_input;
		std::string filtered_input;
		std::string filtered;

		if(!real_encoding.empty()) {
			if(!encoding::is_ascii_compatible(real_encoding)) { 
				work_encoding = "UTF-8";
				repl_ch = 0;
				try {
					std::string tmp = 
						booster::locale::conv::to_utf<char>(
							begin,
							end,
							real_encoding,
							booster::locale::conv::stop
							);
					work_input.swap(tmp);
				}
				catch(...) {
					valid = false;
					std::string tmp = 
						booster::locale::conv::to_utf<char>(
							begin,
							end,
							real_encoding,
							booster::locale::conv::skip
							);
					work_input.swap(tmp);
				}
				begin = work_input.c_str();
				end = begin + work_input.size();
			}

			if(!encoding::validate_or_filter(work_encoding,begin,end,filtered_input,repl_ch)) {
				valid = false;
				begin = filtered_input.c_str();
				end = begin + filtered_input.size();
			}
		}

		std::vector<entry> parsed;
		
		split_to_parts(begin,end,parsed);

		size_t size = parsed.size();

		for(unsigned i=0;i<size;i++) {
			if(parsed[i].type == invalid_data)
				valid = false;
			parse_part(parsed[i]);
			if(parsed[i].type == invalid_data)
				valid = false;
		}

		validate_nesting(parsed,r.html() == rules::xhtml_input);

		for(unsigned i=0;i<size;i++)
			if(parsed[i].type == invalid_data)
				valid = false;

		for(unsigned i=0;i<size;i++) {
			if(!validate_entry_by_rules(parsed[i],r)) {
				valid = false;
				int pair = parsed[i].tag.pair;
				if(pair!=-1)
					parsed[pair].type = invalid_data;
				parsed[i].type = invalid_data;
			}
		}
		if(valid)
			return true;
		
		filtered.clear();
		filtered.reserve(end-begin);
		
		for(unsigned i=0;i<size;i++) {
			char const *b=parsed[i].begin;
			char const *e=parsed[i].end;
			if(parsed[i].type == invalid_data) {
				if(method == remove_invalid)
					continue;
				for(char const *p=b;p!=e;p++) {
					char c=*p;
					switch(c) {
					case '<':
						filtered+="&lt;";
						break;
					case '>':
						filtered+="&gt;";
						break;
					case '&':
						filtered+="&amp;";
						break;
					case '"':
						filtered+="&quot;";
						break;
					default:
						filtered+=c;
					}
				}
			}
			else {
				filtered.append(b,e-b);
			}
		}

		if(work_encoding == real_encoding) {
			output.swap(filtered);
		}
		else {
			try {
				std::string tmp = booster::locale::conv::from_utf<char>(
						filtered,
						real_encoding,
						booster::locale::conv::stop);
				output.swap(tmp);
			}
			catch(...) {
				// Nothing to do
			}
		}
		return false;
	}
	
	std::string filter(	char const *begin,
				char const *end,
				rules const &r,
				filtering_method_type method,
				char repl_ch)
	{
		std::string res;
		if(validate_and_filter_if_invalid(begin,end,r,res,method,repl_ch)) {
			res.assign(begin,end-begin);
		}
		return res;
	}
	std::string filter(	std::string const &input,
				rules const &r,
				filtering_method_type method,
				char repl_ch)
	{
		char const *begin = input.c_str();
		char const *end = begin + input.size();
		std::string res;
		if(validate_and_filter_if_invalid(begin,end,r,res,method,repl_ch)) {
			return input;
		}
		else
			return res;
	}


} } // cppcms::xss
