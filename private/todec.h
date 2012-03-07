///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_TODEC_H
#define CPPCMS_IMPL_TODEC_H
#include <limits>
#include <ostream>

namespace cppcms {
	namespace impl {
		namespace details {
			template<bool sig>
			struct decimal_traits;

			template<>
			struct decimal_traits<false> {
				template<typename T>
				static void conv(T v,char *&begin,char *&buf)
				{
					begin = buf;
					while(v!=0) {
						*buf++ = '0' + v % 10;
						v/=10;
					}
				}
			};

			template<>
			struct decimal_traits<true> {
				template<typename T>
				static void conv(T v,char *&begin,char *&buf)
				{
					if(v<0) {
						*buf++ = '-';
						begin=buf;
						while(v!=0) {
							*buf++ = '0' - (v % 10);
							v/=10;
						}
					}
					else {
						decimal_traits<false>::conv(v,begin,buf);
					}
				}
			};
		}


		template<typename Integer>
		void todec(Integer v,char *buf)
		{
			typedef std::numeric_limits<Integer> limits;
			if(v == 0) {
				*buf++ = '0';
				*buf++ = 0;
				return;
			}
			char *begin=0;

			details::decimal_traits<limits::is_signed>::conv(v,begin,buf);
			
			*buf-- = 0;
			while(begin < buf) {
				char tmp = *begin;
				*begin = *buf;
				*buf = tmp;
				buf--;
				begin++;
			}
		}

		template<typename I>
		std::string todec_string(I v)
		{
			char buf[std::numeric_limits<I>::digits10 + 4];
			todec<I>(v,buf);
			std::string tmp = buf;
			return tmp;
		}

		namespace details {
			template<typename T>
			struct write_int_to_stream {
				write_int_to_stream(T vi = 0) : v(vi) {}
				T v;
				void operator()(std::ostream &out) const
				{
					char buf[std::numeric_limits<T>::digits10 + 4];
					todec<T>(v,buf);
					out << buf;
				}
			};
			template<typename T>
			std::ostream &operator<<(std::ostream &out,write_int_to_stream<T> const &v)
			{
				v(out);
				return out;
			}
		} // details

		template<typename T>
		details::write_int_to_stream<T> cint(T v)
		{
			return details::write_int_to_stream<T>(v);
		}
	}
}
#endif
