//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_MESSAGE_HPP_INCLUDED
#define CPPCMS_LOCALE_MESSAGE_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include <locale>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include "locale_formatting.h"

namespace cppcms {
    namespace locale {

        class info;
        ///
        /// \brief Hook class for DLL
        ///
        /// This class is big ugly hook for DLL in order to make sure that both program and DLL
        /// refer to same locale::id when it uses some atomic static members.
        ///
        /// Further we specialize it for char, wchar_t, char16_t and char32_t in order to make them work.
        ///
        template<typename CharType>
        struct base_message_format: public std::locale::facet
        {
        };

       
       
        ///
        /// \brief A facet used for message formatting
        ///
        /// This class is usually used via translate function
        /// 
        template<typename CharType>
        class message_format : public base_message_format<CharType>
        {
        public:

            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;

            message_format(size_t refs = 0) : 
                base_message_format<CharType>(refs)
            {
            }

            virtual char_type const *get(int domain_id,char const *id) const = 0;
            virtual char_type const *get(int domain_id,char const *single_id,int n) const = 0;
            virtual int domain(std::string const &domain) const = 0;

#if defined (__SUNPRO_CC) && defined (_RWSTD_VER)
            std::locale::id& __get_id (void) const { return id; }
#endif
            static message_format<CharType> *create(info const &,std::vector<std::string> const &paths,std::vector<std::string> const &domains);

        protected:
            virtual ~message_format()
            {
            }

        };


        ///
        /// This class represents a message that can be converted to specific locale message
        ///
        struct message {
        public:

            ///
            /// Create default empty message
            /// 
            message() :
                n_(0),
                c_id_(""),
                c_plural_(0)
            {
            }

            ///
            /// Create a simple message from 0 terminated string. The string should exist
            /// until message is destroyed. Generally useful with static constant strings
            /// 
            explicit message(char const *id) :
                n_(0),
                c_id_(id),
                c_plural_(0)
            {
            }

            ///
            /// Create a simple plural form message from 0 terminated strings. The strings should exist
            /// until message is destroyed. Generally useful with static constant strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            explicit message(char const *single,char const *plural,int n) :
                n_(n),
                c_id_(single),
                c_plural_(plural)
            {
            }

            ///
            /// Create a simple message from string.
            ///
            explicit message(std::string const &id) :
                n_(0),
                c_id_(0),
                c_plural_(0),
                id_(id)
            {
            }

            ///
            /// Create a simple plural form message from strings.
            ///
            /// \a n is the number, \a single and \a plural are single and plural forms of message
            /// 
            explicit message(std::string const &single,std::string const &plural,int number) :
                n_(number),
                c_id_(0),
                c_plural_(0),
                id_(single),
                plural_(plural)
            {
            }

            ///
            /// Message class can be explicitly converter to string class
            ///

            template<typename CharType>
            operator std::basic_string<CharType> () const
            {
                return str<CharType>();
            }

            ///
            /// Translate message to the string in default global locale, using default domain
            ///
            template<typename CharType>
            std::basic_string<CharType> str() const
            {
                std::locale loc;
                return str<CharType>(loc,0);
            }
            
            ///
            /// Translate message to string in the locale \a locale, using default domain
            ///
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &locale) const
            {
                return str<CharType>(locale,0);
            }
           
            ///
            /// Translate message to string using locale \a locale and message domain  \ a domain_id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &locale,std::string domain_id) const
            {
                int id=0;
                if(std::has_facet<message_format<CharType> >(locale))
                    id=std::use_facet<message_format<CharType> >(locale).domain(domain_id);
                return str<CharType>(locale,id);
            }

            ///
            /// Translate message to string using defailt locale message domain  \ a domain_id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::string domain_id) const
            {
                int id=0;
                std::locale locale;
                if(std::has_facet<message_format<CharType> >(locale))
                    id=std::use_facet<message_format<CharType> >(locale).domain(domain_id);
                return str<CharType>(locale,id);
            }

            
            ///
            /// Translate message to string using locale \a loc and message domain index  \ a id
            /// 
            template<typename CharType>
            std::basic_string<CharType> str(std::locale const &loc,int id) const
            {
                std::basic_string<CharType> buffer;                
                CharType const *ptr = write(loc,id,buffer);
                if(ptr == buffer.c_str())
                    return buffer;
                else
                    buffer = ptr;
                return buffer;
            }


            ///
            /// Translate message and write to stream \a out, using imbued locale and domain set to the 
            /// stream
            ///
            template<typename CharType>
            void write(std::basic_ostream<CharType> &out) const
            {
                std::locale const &loc = out.getloc();
                int id = ext_value(out,flags::domain_id);
                std::basic_string<CharType> buffer;
                out << write(loc,id,buffer);
            }

        private:
            
            template<typename CharType>
            CharType const *write(std::locale const &loc,int domain_id,std::basic_string<CharType> &buffer) const
            {
                CharType const *translated = 0;
                static const CharType empty_string[1] = {0};

                char const *id = c_id_ ? c_id_ : id_.c_str();
                char const *plural = c_plural_ ? c_plural_ : (plural_.empty() ? 0 : plural_.c_str());
                
                if(*id == 0)
                    return empty_string;
                
                if(std::has_facet<message_format<CharType> >(loc)) {
                    message_format<CharType> const &msg = std::use_facet<message_format<CharType> >(loc);
                    
                    if(!plural) {
                        translated = msg.get(domain_id,id);
                    }
                    else {
                        translated = msg.get(domain_id,id,n_);
                    }
                }

                if(!translated) {
                    char const *msg = plural ? ( n_ == 1 ? cut_comment(id) : plural) : cut_comment(id);

                    std::ctype<CharType> const &ct = std::use_facet<std::ctype<CharType> >(loc);
                    while(*msg)
                        buffer+=ct.widen(*msg++);

                    translated = buffer.c_str();
                }
                return translated;
            }

            char const *cut_comment(char const *id) const
            {
                static const char key = '#';
                if(*id == key) {
                    if(id[1] == key)
                        return id+1;
                    char c;
                    id++;
                    while((c=*id)!=0 && c!=key)
                        id++;
                    if(*id==key)
                        id++;
                }
                return id;
            }

            /// members

            int n_;
            char const *c_id_;
            char const *c_plural_;
            std::string id_;
            std::string plural_;
        };

        ///
        /// Translate message \a msg and write it to stream
        ///
        template<typename CharType>
        std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out,message const &msg)
        {
            msg.write(out);
            return out;
        }


        ///
        /// Translate a message, \a msg is not copied 
        ///
        inline message translate(char const *msg)
        {
            return message(msg);
        }
        ///
        /// Translate a plural message from, \a single and \a plural are not copied 
        ///
        inline message translate(char const *single,char const *plural,int n)
        {
            return message(single,plural,n);
        }
        
        ///
        /// Translate a message, \a msg is copied 
        ///
        inline message translate(std::string const &msg)
        {
            return message(msg);
        }
        ///
        /// Translate a plural message from, \a single and \a plural are copied 
        ///
        inline message translate(std::string const &single,std::string const &plural,int n)
        {
            return message(single,plural,n);
        }
        
        template<>
        struct CPPCMS_API base_message_format<char> : public std::locale::facet 
        {
            base_message_format(size_t refs = 0) : std::locale::facet(refs)
            {
            }
            static std::locale::id id;
        };
        
        template<>
        CPPCMS_API message_format<char> *message_format<char>::create(   info const &,
                                                                                std::vector<std::string> const &domains,
                                                                                std::vector<std::string> const &paths);
        namespace as {
            namespace details {
                struct set_domain {
                    std::string domain_id;
                };
                template<typename CharType>
                std::basic_ostream<CharType> &operator<<(std::basic_ostream<CharType> &out, set_domain const &dom)
                {
                    int id = std::use_facet<message_format<CharType> >(out.getloc()).domain(dom.domain_id);
                    ext_value(out,flags::domain_id,id);
                    return out;
                }
            } // details

            ///
            /// Manipulator for switching domain in ostream,
            ///
            inline details::set_domain domain(std::string const &id)
            {
                details::set_domain tmp;
                tmp.domain_id = id;
                return tmp;
            }
        } // as
    } // locale 
} // boost


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

