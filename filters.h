#ifndef CPPCMS_FILTERS_H
#define CPPCMS_FILTERS_H

#include <locale>
#include <typeinfo>
#include <sstream>
#include <vector>
#include <iostream>
#include "defs.h"
#include "copy_ptr.h"

#include "locale_gettext.h"
namespace cppcms {
	namespace filters {

		#define CPPCMS_STREAMED(Streamable)						\
		inline std::ostream &operator<<(std::ostream &out,Streamable const &object)	\
		{										\
			object(out);								\
			return out;								\
		};

		

		class CPPCMS_API streamable {
		public:
			typedef void (*to_stream_type)(std::ostream &,void const *ptr);
			typedef std::string (*to_string_type)(std::ios &,void const *ptr);
			

			streamable();
			~streamable();
			streamable(streamable const &other);
			streamable const &operator=(streamable const &other);

			template<typename S>
			streamable(S const &ptr)
			{
				set(&ptr,to_stream<S>,to_string<S>);
			}
			
			streamable(char const *ptr);

			void operator()(std::ostream &output) const;
			std::string get(std::ios &ios) const;
		private:
			void set(void const *ptr,to_stream_type f1,to_string_type f2);

			template<typename T>
			static void to_stream(std::ostream &out,void const *ptr)
			{
				T const *object=reinterpret_cast<T const *>(ptr);
				out << *object;
			}
			template<typename T>
			std::string to_string(std::ios &out,void const *ptr)
			{
				T const *object=reinterpret_cast<T const *>(ptr);
				std::ostringstream oss;
				oss.copyfmt(out);
				oss << *object;
				return oss.str();
			}
		private:
			struct data;
			void const *ptr_;
			to_stream_type to_stream_;
			to_string_type to_string_;
			util::copy_ptr<data> d;
		};

		///
		/// \brief we can specialize for string because std::string get(ios &) const may be much more
		/// efficient.
		///
		template<>
		CPPCMS_API streamable::streamable(std::string const &str);


		CPPCMS_STREAMED(streamable)

		///
		/// \brief Output filter to_upper
		///
		/// Convert text to upper case according to locale
		///
		
		class CPPCMS_API to_upper {					
		public:	
			to_upper();						
			~to_upper();					
			to_upper(to_upper const &);				
			to_upper const &operator=(to_upper const &other);	
			void operator()(std::ostream &out) const;
			to_upper(streamable const &obj);

		private:						
			streamable obj_;			
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,to_upper const &obj)
		{
			obj(out);
			return out;
		}
	
		///
		/// \brief Output filter to_lower
		///
		/// Convert text to lower case according to locale
		///
		
		class CPPCMS_API to_lower {					
		public:	
			to_lower();						
			~to_lower();					
			to_lower(to_lower const &);				
			to_lower const &operator=(to_lower const &other);	
			void operator()(std::ostream &out) const;
			to_lower(streamable const &obj);

		private:						
			streamable obj_;			
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,to_lower const &obj)
		{
			obj(out);
			return out;
		}
		
		///
		/// \brief Output filter to_title
		///
		/// Convert text to title case according to locale
		///
		
		class CPPCMS_API to_title {					
		public:	
			to_title();						
			~to_title();					
			to_title(to_title const &);				
			to_title const &operator=(to_title const &other);	
			void operator()(std::ostream &out) const;
			to_title(streamable const &obj);

		private:						
			streamable obj_;			
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,to_title const &obj)
		{
			obj(out);
			return out;
		}
	
		///
		/// \brief Output filter escape
		///
		/// Escape text for HTML -- make text safe
		///
		
		class CPPCMS_API escape {					
		public:	
			escape();						
			~escape();					
			escape(escape const &);				
			escape const &operator=(escape const &other);	
			void operator()(std::ostream &out) const;
			escape(streamable const &obj);

		private:						
			streamable obj_;			
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,escape const &obj)
		{
			obj(out);
			return out;
		}


		///
		/// \brief Output filter urlencode
		///
		/// Perform urlencoding(percent encoding) of the text
		///
		
		class CPPCMS_API urlencode {					
		public:	
			urlencode();						
			~urlencode();					
			urlencode(urlencode const &);				
			urlencode const &operator=(urlencode const &other);	
			void operator()(std::ostream &out) const;
			urlencode(streamable const &obj);

		private:						
			streamable obj_;			
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,urlencode const &obj)
		{
			obj(out);
			return out;
		}
	
		///
		/// \brief Output filter base64_urlencode
		///
		/// Convert text to base64 format that is friendly to URL.
		///
		
		class CPPCMS_API base64_urlencode {					
		public:	
			base64_urlencode();						
			~base64_urlencode();					
			base64_urlencode(base64_urlencode const &);				
			base64_urlencode const &operator=(base64_urlencode const &other);	
			void operator()(std::ostream &out) const;
			base64_urlencode(streamable const &obj);

		private:						
			streamable obj_;			
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,base64_urlencode const &obj)
		{
			obj(out);
			return out;
		}
		
		///
		/// \brief Output filter raw
		///
		/// Filter that does nothing
		///
		
		class CPPCMS_API raw {					
		public:	
			raw();						
			~raw();					
			raw(raw const &);				
			raw const &operator=(raw const &other);	
			void operator()(std::ostream &out) const;
			raw(streamable const &obj);

		private:						
			streamable obj_;			
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,raw const &obj)
		{
			obj(out);
			return out;
		}
	
		///
		/// \brief Output filter strftime
		///
		/// Format time according to locale using time_put<char> facet. (has similar
		/// parameters to C strftime
		///
		
		class CPPCMS_API strftime {					
		public:	
			strftime();						
			~strftime();					
			strftime(strftime const &);				
			strftime const &operator=(strftime const &other);	
			void operator()(std::ostream &out) const;
			strftime(streamable const &obj,std::tm const &t);

		private:						
			streamable format_;
			std::tm const *t_;
			struct data;					
			util::copy_ptr<data> d;				
		};

		inline std::ostream &operator<<(std::ostream &out,strftime const &obj)
		{
			obj(out);
			return out;
		}
	

		class CPPCMS_API date {
		public:
			date();
			date(date const &other);
			date const &operator=(date const &other);
			~date();

			date(std::tm const &t);
			void operator()(std::ostream &out) const;
		private:
			struct data;
			std::tm const *t_;
			util::copy_ptr<data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,date const &obj)
		{
			obj(out);
			return out;
		}
		
		class CPPCMS_API time {
		public:
			time();
			time(time const &other);
			time const &operator=(time const &other);
			~time();

			time(std::tm const &t);
			void operator()(std::ostream &out) const;
		private:
			struct data;
			std::tm const *t_;
			util::copy_ptr<data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,time const &obj)
		{
			obj(out);
			return out;
		}
		class CPPCMS_API datetime {
		public:
			datetime();
			datetime(datetime const &other);
			datetime const &operator=(datetime const &other);
			~datetime();

			datetime(std::tm const &t);
			void operator()(std::ostream &out) const;
		private:
			struct data;
			std::tm const *t_;
			util::copy_ptr<data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,datetime const &obj)
		{
			obj(out);
			return out;
		}
	
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
