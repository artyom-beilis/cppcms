//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_FORMAT_HPP_INCLUDED
#define CPPCMS_LOCALE_FORMAT_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include "locale_message.h"
#include "locale_formatting.h"

#include <sstream>
#include <iostream>


namespace cppcms {
    namespace locale {
        
        ///
        /// \defgroup format Format
        ///
        /// This module provides printf like functionality integrated to iostreams and suitable for localization
        ///
        /// @{
        ///

        /// \cond INTERNAL
        namespace details {

            template<typename CharType>
            struct formattible {
                typedef std::basic_ostream<CharType> stream_type;
                typedef void (*writer_type)(stream_type &output,void const *ptr);

                formattible() :
                    pointer_(0),
                    writer_(&formattible::void_write)
                {
                }
                
                formattible(formattible const &other) :
                    pointer_(other.pointer_),
                    writer_(other.writer_)
                {
                }

                formattible const &operator=(formattible const &other)
                {
                    if(this != &other) {
                        pointer_=other.pointer_;
                        writer_=other.writer_;
                    }
                    return *this;
                }
                
                template<typename Type>
                formattible(Type const &value)
                {
                    pointer_ = reinterpret_cast<void const *>(&value);
                    writer_ = &write<Type>;
                }

                template<typename Type>
                formattible const &operator=(Type const &other)
                {
                    *this = formattible(other);
                    return *this;
                }

                friend stream_type &operator<<(stream_type &out,formattible const &fmt)
                {
                    fmt.writer_(out,fmt.pointer_);
                    return out;
                }

            private:
                static void void_write(stream_type &output,void const *ptr)
                {
                    CharType empty_string[1]={0};
                    output<<empty_string;
                }

                template<typename Type>
                static void write(stream_type &output,void const *ptr)
                {
                    output << *reinterpret_cast<Type const *>(ptr);
                }
                
                void const *pointer_;
                writer_type writer_;
            }; // formattible
    
            class CPPCMS_API format_parser  {
            public:
                format_parser(std::ios_base &ios);
                ~format_parser();
                
                unsigned get_posision();
                
                void set_one_flag(std::string const &key,std::string const &value);

                template<typename CharType>
                void set_flag_with_str(std::string const &key,std::basic_string<CharType> const &value)
                {
                    if(key=="ftime" || key=="strftime") {
                        as::strftime(ios_);
                        ext_pattern(ios_,flags::datetime_pattern,value);
                    }
                }
                void restore();
            private:
                format_parser(format_parser const &);
                void operator=(format_parser const &);

                std::ios_base &ios_;
                struct data;
                std::auto_ptr<data> d;
            };

        }

        /// \endcond

        ///
        /// \brief a printf like class that allows type-safe and locale aware message formatting
        ///
        /// This class creates formatted message similarly to printf or boost::format and receives
        /// formatted entries via operator %.
        ///
        /// For example
        /// \code
        ///  cout << format("Hello {1}, you are {2} years old") % name % age << endl;
        /// \endcode
        ///
        /// Formatting is enclosed between curl brackets \c { \c }  and defined by comma separated list of flags in format key[=value]
        /// value may also be text included between single quotes \c ' that is used for special purposes where inclusion of non-ASCII
        /// text is allowed
        ///
        /// For example:
        ///
        /// \code 
        ///   cout << format("The hight of water at {1,time} is {2,num=fixed,precision=3}") % time % height;
        /// \endcode
        ///
        /// The special key -- number without value defines a position of input parameter.
        /// List of keys:
        /// -   \c [0-9]+ -- digits, the index of formatted parameter -- mandatory key.
        /// -   \c num or \c number -- format a number. Optional values are:
        ///     -  \c hex -- display hexadecimal number
        ///     -  \c oct -- display in octal format
        ///     -  \c sci or `scientific` -- display in scientific format
        ///     -  \c fix or `fixed` -- display in fixed format
        ///     .      
        ///     For example \c number=sci
        /// -  \c cur or \c currency -- format currency. Optional values are:
        /// 
        ///     -  \c iso -- display using ISO currency symbol.
        ///     -  \c nat or \c national -- display using national currency symbol.
        ///     .
        /// -  \c per or \c percent -- format percent value.
        /// -  \c date, \c time , \c datetime or \c dt -- format date, time or date and time. Optional values are:
        ///     -  \c s or \c short -- display in short format
        ///     -  \c m or \c medium -- display in medium format.
        ///     -  \c l or \c long -- display in long format.
        ///     -  \c f or \c full -- display in full format.
        ///     .
        /// -  \c ftime with string (quoted) parameter -- display as with \c strftime see, \c as::ftime manipulator
        /// -  \c spell or \c spellout -- spell the number.
        /// -  \c ord or \c ordinal -- format ordinal number (1st, 2nd... etc)
        /// -  \c left or \c < -- align to left.
        /// -  \c right or \c > -- align to right.
        /// -  \c width or \c w -- set field width (requires parameter).
        /// -  \c precision or \c p -- set precision (requires parameter).
        /// -  \c locale -- with parameter -- switch locale for current operation. This command generates locale
        ///     with formatting facets giving more fine grained control of formatting. For example:
        /// 
        /// 
        /// 
        template<typename CharType>
        class basic_format {
        public:
            typedef CharType char_type; ///< Underlying character type
            /// \cond INTERNAL
            typedef details::formattible<CharType> formattible_type; 
            /// \endcond 

            typedef std::basic_string<CharType> string_type; ///< string type for this type of character
            typedef std::basic_ostream<CharType> stream_type; ///< output stream type for this type of character
 

            ///
            /// Create a format class for \a format_string
            ///
            basic_format(string_type format_string) : 
                format_(format_string),
                translate_(false),
                parameters_count_(0)
            {
            }
            ///
            /// Create a format class using message \a trans. The message if translated first according
            /// to the rules of target locale and then interpreted as format string
            ///
            basic_format(message const &trans) : 
                message_(trans),
                translate_(true),
                parameters_count_(0)
            {
            }

            ///
            /// Add new parameter to format list
            ///
            template<typename Formattible>
            basic_format &operator % (Formattible const &object)
            {
                add(formattible_type(object));
                return *this;
            }

            ///
            /// Format a string using a locale \a loc
            ///
            string_type str(std::locale const &loc = std::locale()) const
            {
                std::basic_ostringstream<CharType> buffer;
                buffer.imbue(loc);
                write(buffer,false);
                return buffer.str();
            }

            ///
            /// write a formatted string to output stream \a out using out's locale
            ///
            void write(stream_type &out) const
            {
                string_type format;
                if(translate_)
                    format = message_.str<CharType>(out.getloc(),ext_value(out,flags::domain_id));
                else
                    format = format_;
               
                format_output(out,format);

            }
                        
            
        private:
            
            void format_output(stream_type &out,string_type const &sformat) const
            {
                char_type obrk='{';
                char_type cbrk='}';
                char_type eq='=';
                char_type comma=',';
                char_type quote='\'';

                size_t pos = 0;
                size_t size=sformat.size();
                CharType const *format=sformat.c_str();
                while(format[pos]!=0) {
                    if(format[pos] != obrk) {
                        if(format[pos]==cbrk && format[pos+1]==cbrk) {
                            out << cbrk;
                            pos+=2;
                        }
                        else {
                            out<<format[pos];
                            pos++;
                        }
                        continue;
                    }

                    if(pos+1 < size && format[pos+1]==obrk) {
                        out << obrk;
                        pos+=2;
                        continue;
                    }
                    pos++;
                   
                    
                    details::format_parser fmt(out);

                    while(pos < size) { 
                        std::string key;
                        std::string svalue;
                        string_type value;
                        bool use_svalue = true;
                        for(;format[pos];pos++) {
                            char_type c=format[pos];
                            if(c==comma || c==eq || c==cbrk)
                                break;
                            else {
                                key+=static_cast<char>(c);
                            }
                        }

                        if(format[pos]==eq) {
                            pos++;
                            if(format[pos]==quote) {
                                pos++;
                                use_svalue = false;
                                while(format[pos]) {
                                    if(format[pos]==quote) {
                                        if(format[pos+1]==quote) {
                                            value+=quote;
                                            pos+=2;
                                        }
                                        else {
                                            pos++;
                                            break;
                                        }
                                    }
                                    else {
                                        value+=format[pos];
                                        pos++;
                                    }
                                }
                            }
                            else {
                                char_type c;
                                while((c=format[pos])!=0 && c!=comma && c!=cbrk) {
                                    svalue+=static_cast<char>(c);
                                    pos++;
                                }
                            }
                        }

                        if(use_svalue)
                            fmt.set_one_flag(key,svalue);
                        else 
                            fmt.set_flag_with_str(key,value);
                        
                        if(format[pos]==',') {
                            pos++;
                            continue;
                        }
                        else if(format[pos]=='}')  {
                            unsigned position = fmt.get_posision();
                            out << get(position);
                            fmt.restore();
                            pos++;
                            break;
                        }
                        else {                        
                            fmt.restore();
                            break;
                        }
                    }
                }
            }

      
            //
            // Non-copyable 
            //
            basic_format(basic_format const &other);
            void operator=(basic_format const &other);

            void add(formattible_type const &param)
            {
                if(parameters_count_ >= base_params_)
                    ext_params_.push_back(param);
                else 
                    parameters_[parameters_count_] = param;
                parameters_count_++;
            }

            formattible_type get(unsigned id) const
            {
                if(id >= parameters_count_)
                    return formattible_type();
                else if(id >= base_params_)
                    return ext_params_[id - base_params_];
                else
                    return parameters_[id];
            }



            static unsigned const base_params_ = 8;
            
            message message_;
            string_type format_;
            bool translate_;


            formattible_type parameters_[base_params_];
            unsigned parameters_count_;
            std::vector<formattible_type> ext_params_;
        };

        ///
        /// Write formatted message to stream.
        ///
        /// This operator actually causes actual text formatting. It use locale and of \a out stream
        ///
        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,basic_format<CharType> const &fmt)
        {
            fmt.write(out);
            return out;
        }


        ///
        /// Definiton of char based format
        ///
        typedef basic_format<char> format;

        #ifndef CPPCMS_NO_STD_WSTRING
        ///
        /// Definiton of wchar_t based format
        ///
        typedef basic_format<wchar_t> wformat;
        #endif

        #ifdef CPPCMS_HAS_CHAR16_T
        ///
        /// Definiton of char16_t based format
        ///
        typedef basic_format<char16_t> u16format;
        #endif

        #ifdef CPPCMS_HAS_CHAR32_T
        ///
        /// Definiton of char32_t based format
        ///
        typedef basic_format<char32_t> u32format;
        #endif

        ///
        /// @}
        ///

    }
}


#endif

///
/// \example hello.cpp
///
/// Basic example of using various functions provided by this library
///
/// \example whello.cpp
///
/// Basic example of using various functions with wide strings provided by this library
///
///

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

