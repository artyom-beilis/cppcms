//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_SRC_LOCALE_FORMATTING_INFO_HPP_INCLUDED
#define CPPCMS_SRC_LOCALE_FORMATTING_INFO_HPP_INCLUDED


#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/config.hpp>
#else // Internal Boost
#   include <cppcms_boost/config.hpp>
    namespace boost = cppcms_boost;
#endif
#include "locale_formatting.h"
#include "locale_formatter.h"
#include "cstdint.h"
#include <string>
#include <ios>

#include "locale_src_ios_prop.hpp"

namespace cppcms {
    namespace locale {
        namespace impl {

            struct string_set {
                template<typename Char> 
                void set(std::basic_string<Char> const &s);

                template<typename Char> 
                std::basic_string<Char> get() const;

                template<typename Char>
                string_set const &operator=(std::basic_string<Char> const &s)
                {
                    set(s);
                    return *this;
                }

            public:

                std::string str_;
            };

            template<> 
            inline void string_set::set(std::string const &s)
            {
                str_=s;
            }

            template<> 
            inline std::string string_set::get() const
            {
                return str_;
            }

            struct ios_info {
            public:
                ios_info()
                {
                    valid_=false;
                    flags_ = 0;
                    domain_id_=0;
                }
                
                /// never copy formatter
                ios_info(ios_info const &other) :
                    domain_id_(other.domain_id_),
                    flags_(other.flags_),
                    datetime_(other.datetime_),
                    separator_(other.separator_),
                    timezone_(other.timezone_),
                    valid_(false)
                {
                }

                /// never copy formatter
                ios_info const &operator=(ios_info const &other)
                {
                    if(this!=&other) {
                        domain_id_ = other.domain_id_;
                        flags_=other.flags_;
                        datetime_=other.datetime_;
                        separator_=other.separator_;
                        timezone_=other.timezone_;
                        valid_=false;
                    }
                    return *this;
                }

                int value(flags::value_type id) const
                {
                    switch(id) {
                    case flags::domain_id:
                        return domain_id_;
                    default:
                        return 0;
                    }
                }

                void value(flags::value_type id,int value)
                {
                    switch(id) {
                    case flags::domain_id:
                        domain_id_=value;
                        break;
                    default:
                        ;
                    }
                }

                uint64_t flags() const
                {
                    return flags_;
                }
                void flags(uint64_t f)
                {
                    valid_ = valid_ ? flags_ == f : false;
                    flags_=f;
                }

                template<typename Char>
                void datetime(std::basic_string<Char> const &str)
                {
                    valid_ = valid_ ? datetime_.get<Char>() == str : false;
                    datetime_=str;
                }

                template<typename Char>
                std::basic_string<Char> datetime() const
                {
                    return datetime_.get<Char>();
                }
                
                void timezone(std::string const &str)
                {
                    valid_ = valid_ ? timezone_ == str : false;
                    timezone_=str;
                    valid_ = false;
                }

                std::string timezone() const
                {
                    return timezone_;
                }
                
                bool valid(std::ios_base &ios)
                {
                    if( !valid_
                        || ios_flags_ != ios.flags()
                        || precision_ != ios.precision())
                    {
                        formatter_.reset();
                        return false;
                    }
                    return true;
                }

                template<typename CharType>
                cppcms::locale::formatter<CharType> const *formatter(std::ios_base &ios)
                {
                    if(!valid(ios)) {

                        formatter_  = cppcms::locale::formatter<CharType>::create(ios);

                        precision_  = ios.precision();
                        ios_flags_  = ios.flags();
                        valid_      = true;
                    }

                    return dynamic_cast<cppcms::locale::formatter<CharType> const *>(formatter_.get());
                }

                void on_imbue()
                {
                    valid_=false;
                }


            private:
                int domain_id_;
                std::streamsize precision_;
                std::ios_base::fmtflags ios_flags_;
                uint64_t flags_;
                          
                string_set datetime_;
                string_set separator_; 
                std::string timezone_; 
                bool valid_;

                std::auto_ptr<base_formatter> formatter_;

            };


        }
    }
}



#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

