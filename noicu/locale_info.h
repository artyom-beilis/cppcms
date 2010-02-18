//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_INFO_HPP_INCLUDED
#define CPPCMS_LOCALE_INFO_HPP_INCLUDED
#include <locale>
#include <string>
#include <map>
#include <memory>
#include "defs.h"
#include "config.h"

namespace cppcms {
    namespace locale {

        class info_impl;

        ///
        /// \brief a facet that holds general information about locale
        ///
        /// This facet should be always created in order to make all Boost.Locale functions work
        ///
        class CPPCMS_API info : public std::locale::facet
        {
        public:
            static std::locale::id id;
            
            ///
            /// Creates locale using general locale id and cherset encoding
            /// if posix_id is empty default system locale is used.
            ///
            info(std::string lang,std::string country,std::string variant,std::string encoding,size_t refs = 0);
            
            ///
            /// Get language name
            ///
            std::string language() const;
            ///
            /// Get country name
            ///
            std::string country() const;
            ///
            /// Get locale variant
            ///
            std::string variant() const;
            ///
            /// Get encoding
            ///
            std::string encoding() const;

            ///
            /// Is underlying encoding is UTF-8 (for char streams and strings)
            ///
            bool utf8() const
            {
                return utf8_;
            }

#if defined (__SUNPRO_CC) && defined (_RWSTD_VER)
            std::locale::id& __get_id (void) const { return id; }
#endif
    protected:
            
            virtual ~info();

        private:
            std::auto_ptr<info_impl> impl_;
            bool utf8_;
        };

    }
}


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
