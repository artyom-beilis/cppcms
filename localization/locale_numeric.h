//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_NUMERIC_HPP_INCLUDED
#define CPPCMS_LOCALE_NUMERIC_HPP_INCLUDED

#include <locale>
#include <string>
#include <ios>
#include <limits>
#include "locale_formatter.h"
#include "locale_formatting.h"
#include <algorithm>

namespace cppcms {
    namespace locale {


        class num_base {
        protected:

            template<typename ValueType>
            static bool use_parent(std::ios_base &ios)
            {
                uint64_t flg = ext_flags(ios) & flags::display_flags_mask;
                if(flg == flags::posix)
                    return true;

                if(!std::numeric_limits<ValueType>::is_integer)
                    return false;
                if(flg == flags::number && (ios.flags() & std::ios_base::basefield) != std::ios_base::dec) {
                    return true;
                }
                return false;
            }
        };

        namespace details {
            template<typename V,int n=std::numeric_limits<V>::digits,bool integer=std::numeric_limits<V>::is_integer>
            struct cast_traits;
            
            template<typename v>
            struct cast_traits<v,7,true> {
                typedef int32_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,8,true> {
                typedef int32_t cast_type;
            };

            template<typename v>
            struct cast_traits<v,15,true> {
                typedef int32_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,16,true> {
                typedef int32_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,31,true> {
                typedef int32_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,32,true> {
                typedef uint32_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,63,true> {
                typedef int64_t cast_type;
            };
            template<typename v>
            struct cast_traits<v,64,true> {
                typedef uint64_t cast_type;
            };
            template<typename V,int u>
            struct cast_traits<V,u,false> {
                typedef double cast_type;
            };

        }

        template<typename CharType>
        class num_format : public std::num_put<CharType>, protected num_base
        {
        public:
            typedef typename std::num_put<CharType>::iter_type iter_type;
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;
            typedef formatter<CharType> formatter_type;

            num_format(size_t refs = 0) : std::num_put<CharType>(refs)
            {
            }
        protected: 
            

            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, double val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long double val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            
            #ifndef CPPCMS_NO_LONG_LONG 
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long long val) const
            {
                return do_real_put(out,ios,fill,val);
            }
            #endif


       private:



            template<typename ValueType>
            iter_type do_real_put (iter_type out, std::ios_base &ios, char_type fill, ValueType val) const
            {
                formatter_type const *formatter = 0;
                
                if(use_parent<ValueType>(ios) || (formatter = formatter_type::get(ios)) == 0) {
                    return std::num_put<char_type>::do_put(out,ios,fill,val);
                }
                
                size_t code_points;
                typedef typename details::cast_traits<ValueType>::cast_type cast_type;
                string_type const &str = formatter->format(static_cast<cast_type>(val),code_points);
                std::streamsize on_left=0,on_right = 0,points = code_points;
                if(points < ios.width()) {
                    std::streamsize n = ios.width() - points;
                    
                    std::ios_base::fmtflags flags = ios.flags() & std::ios_base::adjustfield;
                    
                    //
                    // We do not really know internal point, so we assume that it does not
                    // exists. So according to standard field should be right aligned
                    //
                    if(flags == std::ios_base::internal || flags == std::ios_base::right)
                        on_left = n;
                    on_right = n - on_left;
                }
                while(on_left > 0) {
                    *out++ = fill;
                    on_left--;
                }
                std::copy(str.begin(),str.end(),out);
                while(on_right > 0) {
                    *out++ = fill;
                    on_right--;
                }
                return out;

            }

        };  /// num_format
       
       
        template<typename CharType>
        class num_parse : public std::num_get<CharType>, protected num_base
        {
        protected: 
            typedef typename std::num_get<CharType>::iter_type iter_type;
            typedef std::basic_string<CharType> string_type;
            typedef CharType char_type;
            typedef formatter<CharType> formatter_type;
            typedef std::basic_istream<CharType> stream_type;

            virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,long &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned short &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned int &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned long &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,float &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            virtual iter_type do_get(iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,double &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            virtual iter_type do_get (iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,long double &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            #ifndef CPPCMS_NO_LONG_LONG 
            virtual iter_type do_get (iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,long long &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            virtual iter_type do_get (iter_type in, iter_type end, std::ios_base &ios,std::ios_base::iostate &err,unsigned long long &val) const
            {
                return do_real_get(in,end,ios,err,val);
            }

            #endif
 
        private:
            

            //
            // This is not really efficient solusion, but it works
            //
            template<typename ValueType>
            iter_type do_real_get(iter_type in,iter_type end,std::ios_base &ios,std::ios_base::iostate &err,ValueType &val) const
            {
                formatter_type const *formatter = 0;
                stream_type *stream_ptr = dynamic_cast<stream_type *>(&ios);

                if(!stream_ptr || use_parent<ValueType>(ios) || (formatter = formatter_type::get(ios)) == 0) {
                    return std::num_get<CharType>::do_get(in,end,ios,err,val);
                }

                typedef typename details::cast_traits<ValueType>::cast_type cast_type;
                string_type tmp;
                tmp.reserve(64);

                // just a hard limit
                std::ctype<CharType> const &type=std::use_facet<std::ctype<CharType> >(ios.getloc());
            
                // SunCC does not like space | cntrl
                while(in!=end && (type.is(std::ctype_base::space,*in) || type.is(std::ctype_base::cntrl,*in)) )
                    ++in;

                while(tmp.size() < 4096 && in!=end && *in!='\n') {
                    tmp += *in++;
                }

                cast_type value;
                size_t parsed_chars;

                if((parsed_chars = formatter->parse(tmp,value))==0 || !valid<ValueType>(value)) {
                    err |= std::ios_base::failbit;
                }
                else {
                    val=static_cast<ValueType>(value);
                }

                for(size_t n=tmp.size();n>parsed_chars;n--) {
                    stream_ptr->putback(tmp[n-1]);
                }

                in = iter_type(*stream_ptr);

                if(in==end)
                    err |=std::ios_base::eofbit;
                return in;
            }

            template<typename ValueType,typename CastedType>
            bool valid(CastedType v) const
            {
                typedef std::numeric_limits<ValueType> value_limits;
                typedef std::numeric_limits<CastedType> casted_limits;
                if(v > (value_limits::max)() || v < (value_limits::min)()) {
                    return false;
                }       
                if(value_limits::is_integer == casted_limits::is_integer) {
                    return true;
                }
                if(value_limits::is_integer) { // and casted is not
                    if(static_cast<CastedType>(static_cast<ValueType>(v))!=v)
                        return false;
                }
                return true;
            }
            
        };



    }
}


#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
