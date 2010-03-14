//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CPPCMS_LOCLAE_TEST_LOCALE_TOOLS_HPP
#define CPPCMS_LOCLAE_TEST_LOCALE_TOOLS_HPP

#include "locale_codepage.h"

template<typename Char>
std::basic_string<Char> to_correct_string(std::string const &e,std::locale l)
{
    return cppcms::locale::conv::to_utf<Char>(e,"UTF-8");
}


template<>
inline std::string to_correct_string(std::string const &e,std::locale l)
{
    return cppcms::locale::conv::from_utf(e,l);
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
