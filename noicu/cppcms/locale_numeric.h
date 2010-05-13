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
#include <cppcms/locale_formatting.h>
#include <algorithm>

namespace cppcms {
    namespace locale {

        typedef std::num_put<char> num_format_base;

        class CPPCMS_API num_format : public num_format_base
        {
        public:
            num_format(size_t refs = 0) : num_format_base(refs)
            {
            }
            virtual ~num_format()
            {
            }
        protected: 

            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long val) const
            {
                if(use_parent(ios))
                    return num_format_base::do_put(out,ios,fill,val);
                return put_value(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long val) const
            {
                if(use_parent(ios))
                    return num_format_base::do_put(out,ios,fill,val);
                return put_value(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, double val) const
            {
                if(use_parent(ios))
                    return num_format_base::do_put(out,ios,fill,val);
                return put_value(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long double val) const
            {
                if(use_parent(ios))
                    return num_format_base::do_put(out,ios,fill,val);
                return put_value(out,ios,fill,val);
            }
            
            #ifndef CPPCMS_NO_LONG_LONG 
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, long long val) const
            {
                if(use_parent(ios))
                    return num_format_base::do_put(out,ios,fill,val);
                return put_value(out,ios,fill,val);
            }
            virtual iter_type do_put (iter_type out, std::ios_base &ios, char_type fill, unsigned long long val) const
            {
                if(use_parent(ios))
                    return num_format_base::do_put(out,ios,fill,val);
                return put_value(out,ios,fill,val);
            }
            #endif


        private:
            bool use_parent(std::ios_base &ios) const;
            iter_type put_value(iter_type out,std::ios_base &ios,char_type fill,long double value) const;

        };  /// num_format

    }
}


#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
