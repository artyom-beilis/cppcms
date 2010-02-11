#ifndef CPPCMS_FILTERS_H
#define CPPCMS_FILTERS_H

#include <locale>
#include <typeinfo>
#include <sstream>
#include <vector>
#include <iostream>
#include "defs.h"
#include "copy_ptr.h"
#include "localization.h"

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
				void const *p=&ptr;
				to_stream_type s1=&to_stream<S>;
				to_string_type s2=&to_string<S>;
				std::type_info const *info=&typeid(S);

				set(p,s1,s2,info);
			}

			template<typename T>
			T const &get() const
			{
				if(typeid(T) != type())
					throw std::bad_cast();
				T const *object=reinterpret_cast<T const *>(ptr_);
				return *object;
			}
			
			streamable(char const *ptr);

			void operator()(std::ostream &output) const;
			std::string get(std::ios &ios) const;
		private:
			void set(void const *ptr,to_stream_type f1,to_string_type f2,std::type_info const *type);

			template<typename T>
			static void to_stream(std::ostream &out,void const *ptr)
			{
				T const *object=reinterpret_cast<T const *>(ptr);
				out << *object;
			}


			template<typename T>
			static std::string to_string(std::ios &out,void const *ptr)
			{
				T const *object=reinterpret_cast<T const *>(ptr);
				std::ostringstream oss;
				oss.copyfmt(out);
				oss << *object;
				return oss.str();
			}

			std::type_info const &type() const;
		private:

			void const *ptr_;
			to_stream_type to_stream_;
			to_string_type to_string_;
			std::type_info const *type_;

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
		/// \brief Format date to ouput stream
		///
		/// Formats date to the stream, date is represented as time_t
		///


		class CPPCMS_API date {
		public:
			date();
			date(date const &other);
			date const &operator=(date const &other);
			~date();

			date(double time);
			void operator()(std::ostream &out) const;
		private:
			struct data;
			double time_;
			util::copy_ptr<data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,date const &obj)
		{
			obj(out);
			return out;
		}
		
		///
		/// \brief Format time to ouput stream
		///
		/// Formats time to the stream, time is represented as time_t
		///
		class CPPCMS_API time {
		public:
			time();
			time(time const &other);
			time const &operator=(time const &other);
			~time();

			time(double time);
			void operator()(std::ostream &out) const;
		private:
			struct data;
			double time_;
			util::copy_ptr<data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,time const &obj)
		{
			obj(out);
			return out;
		}
		///
		/// \brief Format date and time to ouput stream
		///
		/// Formats date and time to the stream, date and time is represented as time_t
		///
		class CPPCMS_API datetime {
		public:
			datetime();
			datetime(datetime const &other);
			datetime const &operator=(datetime const &other);
			~datetime();

			datetime(double t);
			void operator()(std::ostream &out) const;
		private:
			struct data;
			double time_;
			util::copy_ptr<data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,datetime const &obj)
		{
			obj(out);
			return out;
		}

		using locale::translate;
		using locale::format;
	
	}

	///////////////////////////////

}

#undef CPPCMS_STREAMED


#endif
