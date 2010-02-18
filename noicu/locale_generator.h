//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_GENERATOR_HPP
#define CPPCMS_LOCALE_GENERATOR_HPP
#include "defs.h"
#include "config.h"
#include <string>
#include <locale>
#include <memory>

namespace cppcms {
    namespace locale {

        ///
        /// \brief the major class used for locale generation
        ///
        /// This class is used for specification of all parameters required for locale generation and
        /// caching. This class const member functions are thread safe if locale class implementation is thread safe.
        ///

        class CPPCMS_API generator {
        public:

            generator();
            ~generator();

            ///
            /// Set encoding used for 8-bit character encoding. Default is system default encoding
            ///
            void octet_encoding(std::string const &encoding);
            ///
            /// Get encoding used for 8-bit character encoding. Default is system default encoding
            ///
            std::string octet_encoding() const;

            
            ///
            /// Add a new domain of messages that would be generated. It should be set in order to enable
            /// messages support.
            ///
            void add_messages_domain(std::string const &domain);
            ///
            /// Set default message domain. If this member was not called, the first added messages domain is used.
            /// If the domain \a domain is not added yet it is added.
            ///
            void set_default_messages_domain(std::string const &domain);

            ///
            /// Remove all added domains from the list
            ///
            void clear_domains();

            ///
            /// Add a search path where dictionaries are looked in.
            /// 
            void add_messages_path(std::string const &path);

            ///
            /// Remove all added paths
            ///
            void clear_paths();

            ///
            /// Remove all cached locales
            ///
            void clear_cache();

            ///
            /// Generate a locale with id \a id and put it in cache
            ///
            void preload(std::string const &id);
            ///
            /// Generate a locale with id \a id, encoding \a encoding and put it in cache
            ///
            void preload(std::string const &id,std::string const &encoding);

            ///
            /// Generate a locale with id \a id
            ///
            std::locale generate(std::string const &id) const;
            ///
            /// Generate a locale with id \a id, encoding \a encoding 
            ///
            std::locale generate(std::string const &id,std::string const &encoding) const;

            ///
            /// Get a locale with id \a id from cache, if not found, generate one
            ///
            std::locale get(std::string const &id) const;
            ///
            /// Get a locale with id \a id and encoding \a encociding from cache, if not found, generate one
            ///
            std::locale get(std::string const &id,std::string const &encoding) const;

            ///
            /// Shortcut to get(id)
            ///
            std::locale operator()(std::string const &id) const
            {
                return get(id);
            }
            ///
            /// Shortcut to get(id,encoding)
            ///
            std::locale operator()(std::string const &id,std::string const &encoding) const
            {
                return get(id,encoding);
            }

        private:

            std::locale add_facets(std::locale const &source) const;

            generator(generator const &);
            void operator=(generator const &);

            struct data;
            std::auto_ptr<data> d;
        };

    }
}


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

