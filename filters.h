#ifndef CPPCMS_FILTERS_H
#define CPPCMS_FILTERS_H

#include <locale>
#include <typeinfo>
#include <sstream>
#include <vector>
#include <iostream>

#include "locale_gettext.h"
#include "locale_convert.h"
#include "util.h"

namespace cppcms {
	namespace filters {

#define CPPCMS_STREAMED(Streamable) 								\
		inline std::ostream &operator<<(std::ostream &out,Streamable const &object)	\
		{										\
			object(out);								\
			return out;								\
		};

		

		struct streamable {

			streamable() : ptr_(0), info_(0), write_func_(0) {}
		
			/// Optimization for special case
			streamable(char const *ptr) :
				ptr_(ptr),
				info_(&typeid(is_char_ptr)),
				write_func_(write_char)
			{
			}

			template<typename Streamable>
			streamable(Streamable const &object) :
				ptr_(&object),
				info_(&typeid(object))
			{
				write_func_=&write<Streamable>;
			}
			
			streamable(streamable const &s) :
				ptr_(s.ptr_),
				info_(s.info_),
				write_func_(s.write_func_)
			{
			}

			std::type_info const &typeinfo() const
			{
				return *info_;
			}
			void operator()(std::ostream &output) const
			{
				write_func_(output,ptr_);
			}

			template<typename T>
			T const &get() const
			{
				return *reinterpret_cast<T const *>(ptr_);
			}

			std::string get(std::ostream &output) const
			{
				if(typeinfo() == typeid(std::string) )
					return get<std::string>();
				if(typeinfo() == typeid(is_char_ptr)) 
					return reinterpret_cast<char const *>(ptr_);

				std::ostringstream oss;
				oss.imbue(output.getloc());
				write_func_(oss,ptr_);
				return oss.str();
			}
		private:
			struct is_char_ptr{};

			template<typename T>
			static void write(std::ostream &out,void const *ptr)
			{
				T const *object=reinterpret_cast<T const *>(ptr);
				out << *object;
			}
			static inline void write_char(std::ostream &out,void const *ptr)
			{
				out<<reinterpret_cast<char const *>(ptr);
			}

			void const *ptr_;
			std::type_info const *info_;
			void (*write_func_)(std::ostream &out,void const *ptr);

		};


		CPPCMS_STREAMED(streamable)
		
		template<std::string (cppcms::locale::convert::*member)(std::string const &) const>
		class to_something {
		public:
			template<typename Streamable>
			to_something(Streamable const &str) : obj_(str)
			{				
			}
			void operator()(std::ostream &output) const
			{
				(std::use_facet<locale::convert>(output.getloc()).*member)(obj_.get(output));
			}

		private:
			streamable obj_;
		};

		typedef to_something<&locale::convert::to_upper> to_upper;
		typedef to_something<&locale::convert::to_lower> to_lower;
		typedef to_something<&locale::convert::to_title> to_title;

		CPPCMS_STREAMED(to_upper)
		CPPCMS_STREAMED(to_lower)
		CPPCMS_STREAMED(to_title)

		class to_normal {
		public:
			template<typename Streamable>
			to_normal(Streamable const &str,locale::convert::norm_type how = locale::convert::norm_default) :
				 obj_(str),
				 how_(how)
			{				
			}
			void operator()(std::ostream &output) const
			{
				std::use_facet<locale::convert>(output.getloc()).to_normal(obj_.get(output),how_);
			}

		private:
			streamable obj_;
			locale::convert::norm_type how_;
		};

		CPPCMS_STREAMED(to_normal)

		class escape {
		public:
			escape(streamable const &obj) : obj_(obj) {}
			void operator()(std::ostream &output) const
			{
				output << util::escape(obj_.get(output));
			}

		private:
			streamable obj_;
		};
		
		CPPCMS_STREAMED(escape)
		
		class urlencode {
		public:
			urlencode(streamable const &obj) : obj_(obj) {}
			void operator()(std::ostream &output) const
			{
				output << util::urlencode(obj_.get(output));
			}

		private:
			streamable obj_;
		};
		
		CPPCMS_STREAMED(urlencode)
		
		class base64_urlencode {
		public:
			base64_urlencode(streamable const &obj) : obj_(obj) {}
			void operator()(std::ostream &output) const;
		private:
			streamable obj_;
		};
		
		CPPCMS_STREAMED(base64_urlencode)

		class raw {
		public:
			raw(streamable const &obj) : obj_(obj) {}
			void operator()(std::ostream &output) const
			{
				output << obj_;
			}

		private:
			streamable obj_;
		};
		
		CPPCMS_STREAMED(raw)

		class strftime {
		public:
			strftime(char const *format,std::tm const &t) : 
				c_format_(format),
				t_(t)
			{
			}

			strftime(streamable const &format,std::tm const &t) :
				c_format_(0),
				format_(format),
				t_(t)
			{
			}

			void operator()(std::ostream &out) const
			{
				char const *begin,*end;
				if(c_format_) {
					begin=end=c_format_;
					while(*end) end++;
				}
				else {
					std::string const fmt=format_.get(out);
					begin=fmt.data();
					end=begin+fmt.size();
				}
				
				std::use_facet<std::time_put<char> >(out.getloc()).put(out,out,' ',&t_,begin,end);
			}
		private:
			char const *c_format_;
			streamable format_;
			std::tm const &t_;
		};

		CPPCMS_STREAMED(strftime)

		class CPPCMS_API date {
		public:
			date(std::tm const &t);
			void operator()(std::ostream &out) const;
		private:
			std::tm const &tm_;
		};
		
		CPPCMS_STREAMED(date)
		
		class CPPCMS_API time {
		public:
			time(std::tm const &t);
			void operator()(std::ostream &out) const;
		private:
			std::tm const &tm_;
		};
		CPPCMS_STREAMED(time)
		
		class CPPCMS_API datetime {
		public:
			datetime(std::tm const &t);
			void operator()(std::ostream &out) const;
		private:
			std::tm const &tm_;
		};
		CPPCMS_STREAMED(datetime)

		class gt {
		public:
			gt(char const *msg) : 
				domain_(0),
				msg_(msg)
			{
			}
			gt(char const *domain,char const *msg) :
				domain_(domain),
				msg_(msg)
			{
			}
			void operator()(std::ostream &out) const
			{
				locale::gettext const &trans = std::use_facet<locale::gettext>(out.getloc());
				if(domain_)
					out << trans.dictionary(domain_).gettext(msg_);
				else
					out << trans.dictionary().gettext(msg_);
			}
		private:
			char const *domain_;
			char const *msg_;
		};
		
		CPPCMS_STREAMED(gt)
		
		class ngt {
		public:
			ngt(char const *s,char const *p,int n) : 
				domain_(0),
				s_(s),
				p_(p),
				n_(n)
			{
			}
			ngt(char const *domain,char const *s,char const *p,int n) :
				domain_(domain),
				s_(s),
				p_(p),
				n_(n)
			{
			}
			void operator()(std::ostream &out) const
			{
				locale::gettext const &trans = std::use_facet<locale::gettext>(out.getloc());
				if(domain_)
					out << trans.dictionary(domain_).ngettext(s_,p_,n_);
				else
					out << trans.dictionary().ngettext(s_,p_,n_);
			}
		private:
			char const *domain_;
			char const *s_,*p_;
			int n_;
		};
		
		CPPCMS_STREAMED(ngt)

		class CPPCMS_API format {
		public:
			format(streamable const &f)
			{
				init(f);
			}
#ifdef CPPCMS_HAVE_VARIADIC_TEMPLATES
			template<typename... Args>
			format(streamable const &f,Args... args)
			{
				init(f);
				add_args(args...);
			}
		private:
			void add_args() 
			{
			}
			template<typename T,typename... Args>
			void add_args(T const &v,Args... args)
			{
				add(v);
				add_args(args...);
			}
		public:
#else			
			template<typename T1>
			format(	streamable const &f,
				T1 const &v1)
			{
				init(f);
				add(v1);
			}			
			template<	typename T1,
					typename T2>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2)
			{
				init(f);
				add(v1);
				add(v2);
			}			
			template<	typename T1,
					typename T2,
					typename T3>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3)
			{
				init(f);
				add(v1);
				add(v2);
				add(v3);
			}			
			template<	typename T1,
					typename T2,
					typename T3,
					typename T4>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3,
				T4 const &v4)
			{
				init(f);
				add(v1);
				add(v2);
				add(v3);
				add(v4);
			}			
			template<	typename T1,
					typename T2,
					typename T3,
					typename T4,
					typename T5>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3,
				T4 const &v4,
				T5 const &v5)
			{
				init(f);
				add(v1);
				add(v2);
				add(v3);
				add(v4);
				add(v5);
			}			
			template<	typename T1,
					typename T2,
					typename T3,
					typename T4,
					typename T5,
					typename T6>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3,
				T4 const &v4,
				T5 const &v5,
				T6 const &v6)
			{
				init(f);
				add(v1);
				add(v2);
				add(v4);
				add(v5);
				add(v6);
			}			
			template<	typename T1,
					typename T2,
					typename T3,
					typename T4,
					typename T5,
					typename T6,
					typename T7>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3,
				T4 const &v4,
				T5 const &v5,
				T6 const &v6,
				T7 const &v7)
			{
				init(f);
				add(v1);
				add(v2);
				add(v3);
				add(v4);
				add(v5);
				add(v6);
				add(v7);
			}			
			template<	typename T1,
					typename T2,
					typename T3,
					typename T4,
					typename T5,
					typename T6,
					typename T7,
					typename T8>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3,
				T4 const &v4,
				T5 const &v5,
				T6 const &v6,
				T7 const &v7,
				T8 const &v8)
			{
				init(f);
				add(v1);
				add(v2);
				add(v3);
				add(v4);
				add(v5);
				add(v6);
				add(v7);
				add(v8);
			}			
			template<	typename T1,
					typename T2,
					typename T3,
					typename T4,
					typename T5,
					typename T6,
					typename T7,
					typename T8,
					typename T9>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3,
				T4 const &v4,
				T5 const &v5,
				T6 const &v6,
				T7 const &v7,
				T8 const &v8,
				T9 const &v9)
			{
				init(f);
				add(v1);
				add(v2);
				add(v3);
				add(v4);
				add(v5);
				add(v6);
				add(v7);
				add(v8);
				add(v9);
			}			
			template<	typename T1,
					typename T2,
					typename T3,
					typename T4,
					typename T5,
					typename T6,
					typename T7,
					typename T8,
					typename T9,
					typename T10>
			format(	streamable const &f,
				T1 const &v1,
				T2 const &v2,
				T3 const &v3,
				T4 const &v4,
				T5 const &v5,
				T6 const &v6,
				T7 const &v7,
				T8 const &v8,
				T9 const &v9,
				T10 const &v10)
			{
				init(f);
				add(v1);
				add(v2);
				add(v3);
				add(v4);
				add(v5);
				add(v6);
				add(v7);
				add(v8);
				add(v9);
				add(v10);
			}
#endif

			template<typename Streamable>
			format &operator % (Streamable const &object)
			{
				return add(streamable(object));
			}

			void operator()(std::ostream &output) const
			{
				write(output);
			}
			
			std::string str(std::locale const &locale) const;

			std::string str() const
			{
				return str(std::locale::classic());
			}

			format &add(streamable const &obj);
		private:
			void write(std::ostream &output) const;
			void format_one(std::ostream &out,std::string::const_iterator &p,std::string::const_iterator e,int &pos) const;

			void init(streamable const &f);

			streamable const *at(size_t n) const;

			streamable format_;
			std::vector<streamable> vobjects_;
			streamable objects_[10];
			size_t size_;
		};

		CPPCMS_STREAMED(format)
	
	}
}

#undef CPPCMS_STREAMED


#endif
