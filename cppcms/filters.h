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
#ifndef CPPCMS_FILTERS_H
#define CPPCMS_FILTERS_H

#include <locale>
#include <typeinfo>
#include <sstream>
#include <vector>
#include <iostream>
#include <cppcms/defs.h>
#include <booster/copy_ptr.h>
#include <cppcms/localization.h>

namespace cppcms {
	///
	/// \brief This namespace various filters that can be used in templates for filtering data
	///
	///
	namespace filters {

		///
		/// This is special object that is used to store a reference to any other object 
		/// that can be written to std::ostream, giving as easy way to write a filter for
		/// any object that can be written to stream
		///
		class CPPCMS_API streamable {
		public:
			/// \cond INTERNAL
			typedef void (*to_stream_type)(std::ostream &,void const *ptr);
			typedef std::string (*to_string_type)(std::ios &,void const *ptr);
			/// \endcond

			streamable();
			~streamable();
			streamable(streamable const &other);
			streamable const &operator=(streamable const &other);

			///
			/// Create a streamable object from arbitrary object that can be written to stream
			///
			template<typename S>
			streamable(S const &ptr)
			{
				void const *p=&ptr;
				to_stream_type s1=&to_stream<S>;
				to_string_type s2=&to_string<S>;
				std::type_info const *info=&typeid(S);

				set(p,s1,s2,info);
			}

			///
			/// Get the reference to "streamed object", throws std::bad_cast if the type is wrong
			///
			template<typename T>
			T const &get() const
			{
				if(typeid(T) != type())
					throw std::bad_cast();
				T const *object=reinterpret_cast<T const *>(ptr_);
				return *object;
			}
			
			///
			/// Create a streamable object from C string
			///
			streamable(char const *ptr);

			///
			/// Write the object to output stream
			///

			void operator()(std::ostream &output) const;

			///
			/// Convert the object to string using settings and locale in ios class
			///
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
			struct _data;					
			booster::copy_ptr<_data> d;				
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
			struct _data;					
			booster::copy_ptr<_data> d;				
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
			struct _data;					
			booster::copy_ptr<_data> d;				
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
			struct _data;					
			booster::copy_ptr<_data> d;				
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
			struct _data;					
			booster::copy_ptr<_data> d;				
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
			struct _data;					
			booster::copy_ptr<_data> d;				
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
			struct _data;					
			booster::copy_ptr<_data> d;				
		
		
		
		};

		inline std::ostream &operator<<(std::ostream &out,raw const &obj)
		{
			obj(out);
			return out;
		}
		
		///
		/// \brief Formats date to the stream, date is represented as number - POSIX time, a plain number
		///
		class CPPCMS_API date {
		public:
			date();
			date(date const &other);
			date const &operator=(date const &other);
			~date();

			///
			/// Create date filter that formats current local-time date using POSIX time representation \a time
			///
			date(streamable const &time);
			///
			/// Create date filter that formats current date using POSIX time representation \a time,
			/// in a timezone \a timezone
			///
			date(streamable const &time,std::string const &timezone);
			void operator()(std::ostream &out) const;
		private:
			struct _data;
			streamable time_;
			std::string tz_;
			booster::copy_ptr<_data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,date const &obj)
		{
			obj(out);
			return out;
		}
		
		///
		/// \brief Format local time to ouput stream
		///
		/// Formats time to the stream, time is represented as number
		///
		class CPPCMS_API time {
		public:
			time();
			time(time const &other);
			time const &operator=(time const &other);
			~time();

			///
			/// Create time filter that formats current local-time time using POSIX time representation \a t
			///
			time(streamable const &t);
			///
			/// Create time filter that formats current time using POSIX time representation \a t,
			/// in a timezone \a timezone
			///
			time(streamable const &time,std::string const &timezone);
			void operator()(std::ostream &out) const;
		private:
			struct _data;
			streamable time_;
			std::string tz_;
			booster::copy_ptr<_data> d;
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

			///
			/// Create date and time filter that formats current local-time date and time using POSIX time representation \a t
			///
			datetime(streamable const &t);
			///
			/// Create date and time filter that formats current date and time using POSIX time representation \a t,
			/// in a timezone \a timezone
			///
			datetime(streamable const &time,std::string const &timezone);
			void operator()(std::ostream &out) const;
		private:
			struct _data;
			streamable time_;
			std::string tz_;
			booster::copy_ptr<_data> d;
		};

		inline std::ostream &operator<<(std::ostream &out,datetime const &obj)
		{
			obj(out);
			return out;
		}
		///
		/// \brief Custom time formating filter
		///
		/// Formats date and time to the stream, date and time is represented as time_t
		///
		class CPPCMS_API strftime {
		public:
			strftime();
			strftime(strftime const &other);
			strftime const &operator=(strftime const &other);
			~strftime();

			///
			/// Create date and time filter that formats current local-time date and time using POSIX time representation \a t
			/// according to strftime like format \a fmt, see booster::locale::as::ftime or cppcms::locale::as::ftime for details 
			/// (depending on your localization backend)
			///
			strftime(streamable const &t,std::string const &fmt);
			///
			/// Create date and time filter that formats current date and time using POSIX time representation \a t,
			/// in a timezone \a timezone
			/// according to strftime like format \a fmt, see booster::locale::as::ftime or cppcms::locale::as::ftime for details 
			/// (depending on your localization backend)
			///
			strftime(streamable const &time,std::string const &timezone,std::string const &fmt);

			void operator()(std::ostream &out) const;
		private:
			struct _data;
			streamable time_;
			std::string tz_;
			std::string format_;
			booster::copy_ptr<_data> d;
		};
		
		inline std::ostream &operator<<(std::ostream &out,strftime const &obj)
		{
			obj(out);
			return out;
		}

		using locale::translate;
		using locale::format;
	
	}

	///////////////////////////////

}


#endif
