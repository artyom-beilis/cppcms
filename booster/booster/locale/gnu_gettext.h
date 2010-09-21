//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCLAE_GNU_GETTEXT_HPP
#define BOOSTER_LOCLAE_GNU_GETTEXT_HPP

#include <booster/locale/message.h>
#include <stdexcept>

namespace booster {
namespace locale {
namespace gnu_gettext {

    ///
    /// \brief This structure holds all information required for creating gnu-gettext message catalogs,
    ///
    /// User is expected to set its parameters to load these catalogs correctly. This sturcture
    /// also allows to provide functions for charset conversion, note, you need to provide them,
    /// so this structure not useful for wide characters without subclassing and it will also
    /// ignore gettext catalogs that use charset different from \a encoding.
    ///
    struct messages_info {
        messages_info() :
            language("C"),
            locale_category("LC_MESSAGES")
        {
        }

        std::string language;   ///< The language we load the catalog for, like "ru", "en", "de" 
        std::string country;    ///< The country wel load the catalog for, like "US", "IL"
        std::string variant;    ///< Language variant, like "euro" so it would look for catalog like de_DE\@euro
        std::string encoding;   ///< Required target charset encoding. Igronred for wide characters.
                                ///< for narror should specify correct encoding required for this catalog
        std::string locale_category; ///< Locale category, is set by default LC_MESSAGES, but ma
        std::vector<std::string> domains;   ///< Message domains - application name, like my_app. So files named my_app.mo
                                            ///< would be loaded
        std::vector<std::string> paths;     ///< Paths to seach files in. 
                                            ///< For Unicode Path under windows, encode the string as UTF-8 and add BOM.

    };

    ///
    /// Create a message_format facet using GNU Gettext catalogs. It uses \a info structure to get
    /// information about where to read them from and uses it for character set conversion (if needed)
    ///

    template<typename CharType>
    message_format<CharType> *create_messages_facet(messages_info &info);

    /// \cond INTERNAL
    
    template<>
    BOOSTER_API message_format<char> *create_messages_facet(messages_info &info);
    
    template<>
    BOOSTER_API message_format<wchar_t> *create_messages_facet(messages_info &info);

    #ifdef BOOSTER_HAS_CHAR16_T
    template<>
    BOOSTER_API message_format<char16_t> *create_messages_facet(messages_info &info);
    #endif
    
    #ifdef BOOSTER_HAS_CHAR32_T
    template<>
    BOOSTER_API message_format<char32_t> *create_messages_facet(messages_info &info);
    #endif

    /// \endcond

} // gnu_gettext
} // locale
} // boost

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

