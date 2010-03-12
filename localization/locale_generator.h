//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
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
    ///
    /// \brief This is the main namespace that encloses all localization classes 
    ///
    namespace locale {

        ///
        /// a enum type that specifies the character type that locales can be generated for
        /// 
        typedef enum {
            char_facet      = 1 << 0,   ///< 8-bit character facets
            wchar_t_facet   = 1 << 1,   ///< wide character facets
            char16_t_facet  = 1 << 2,   ///< C++0x char16_t facets
            char32_t_facet  = 1 << 3,   ///< C++0x char32_t facets

            all_characters = 0xFFFF     ///< Special mask -- generate all
        } character_facet_type;

        ///
        /// a special enum used for more fine grained generation of facets
        ///
        typedef enum {
            collation_facet = 1 << 0,   ///< Generate collation facets
            formatting_facet= 1 << 1,   ///< Generate numbers, currency, date-time formatting facets
            message_facet   = 1 << 2,   ///< Generate message facets
            codepage_facet=   1 << 3,   ///< Generate codepage conversion facets (derived from std::codecvt)
            
            all_categories  = 0xFFFFFFFFu   ///< Generate all of them
        } locale_category_type;

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
            /// Set types of facets that should be generated, default all
            ///
            void categories(unsigned cats);
            ///
            /// Get types of facets that should be generated, default all
            ///
            unsigned categories() const;
            
            ///
            /// Set the characters type for which the facets should be generated, default all supported
            ///
            void characters(unsigned chars);
            ///
            /// Get the characters type for which the facets should be generated, default all supported
            ///
            unsigned characters() const;

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
            /// Generate a locale with id \a id and put it in cache, use \a base as a locale for which all facets are added,
            /// instead of global one
            ///
            void preload(std::locale const &base,std::string const &id);
            ///
            /// Generate a locale with id \a id, encoding \a encoding and put it in cache, use \a base as a locale for which all facets are added,
            /// instead of global one
            ///
            void preload(std::locale const &base,std::string const &id,std::string const &encoding);

            ///
            /// Generate a locale with id \a id
            ///
            std::locale generate(std::string const &id) const;
            ///
            /// Generate a locale with id \a id, encoding \a encoding 
            ///
            std::locale generate(std::string const &id,std::string const &encoding) const;

            ///
            /// Generate a locale with id \a id, use \a base as a locale for which all facets are added,
            /// instead of global one
            ///
            std::locale generate(std::locale const &base,std::string const &id) const;
            ///
            /// Generate a locale with id \a id, encoding \a encoding, use \a base as a locale for which all facets are added,
            /// instead of global one
            ///
            std::locale generate(std::locale const &base,std::string const &id,std::string const &encoding) const;

            ///
            /// Get a locale with id \a id from cache, if not found, generate one
            ///
            std::locale get(std::string const &id) const;
            ///
            /// Get a locale with id \a id and encoding \a encoding from cache, if not found, generate one
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

            template<typename CharType>
            std::locale generate_for(std::locale const &source) const;
            std::locale complete_generation(std::locale const &source) const;

            generator(generator const &);
            void operator=(generator const &);

            struct data;
            std::auto_ptr<data> d;
        };

    }
}


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 

