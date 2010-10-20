//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/locale/conversion.h>
#include <booster/locale/localization_backend.h>
#include <booster/locale/generator.h>
#include <booster/locale/info.h>
#include <iomanip>
#include "test_locale.h"
#include "test_locale_tools.h"
#include "test_posix_tools.h"
#include <iostream>

#include <wctype.h>


template<typename CharType>
void test_one(std::locale const &l,std::string src,std::string tgtl,std::string tgtu)
{
    TEST(booster::locale::to_upper(to_correct_string<CharType>(src,l),l) == to_correct_string<CharType>(tgtu,l));
    TEST(booster::locale::to_lower(to_correct_string<CharType>(src,l),l) == to_correct_string<CharType>(tgtl,l));
    TEST(booster::locale::fold_case(to_correct_string<CharType>(src,l),l) == to_correct_string<CharType>(tgtl,l));
}

template<typename CharType>
void test_char()
{
    booster::locale::generator gen;

    std::cout << "- Testing at least C" << std::endl;

    std::locale l = gen("en_US.UTF-8");

    test_one<CharType>(l,"Hello World i","hello world i","HELLO WORLD I");

    std::string name = "en_US.UTF-8";
    if(have_locale(name)) {
        std::cout << "- Testing " << name << std::endl;
        std::locale l=gen(name);
        test_one<CharType>(l,"Façade","façade","FAÇADE");
    }
    else {
        std::cout << "- en_US.UTF-8 is not supported, skipping" << std::endl;
    }

    name = "en_US.ISO8859-1";
    if(have_locale(name)) {
        std::cout << "Testing " << name << std::endl;
        std::locale l=gen(name);
        test_one<CharType>(l,"Hello World","hello world","HELLO WORLD");
        #ifdef __APPLE__
        if(sizeof(CharType)!=1)
        #endif
            test_one<CharType>(l,"Façade","façade","FAÇADE");
    }
    else {
        std::cout << "- en_US.ISO8859-1 is not supported, skipping" << std::endl;
    }
    
    name = "tr_TR.UTF-8";
    if(have_locale(name)) {
        std::cout << "Testing " << name << std::endl;
        locale_t cl = newlocale(LC_ALL_MASK,name.c_str(),0);
        try { 
            TEST(cl);
            if(towupper_l(L'i',cl) == 0x130) {
                test_one<CharType>(gen(name),"i","i","İ");
            }
            else {
                std::cout <<"  Turkish locale is not supported well" << std::endl;
            }
        }
        catch(...) {
            if(cl) freelocale(cl);
            throw;
        }
        if(cl) freelocale(cl);
        
    }
    else 
    {
        std::cout << "- tr_TR.UTF-8 is not supported, skipping" << std::endl;
    }
}


int main()
{
    try {
        booster::locale::localization_backend_manager mgr = booster::locale::localization_backend_manager::global();
        mgr.select("posix");
        booster::locale::localization_backend_manager::global(mgr);

        std::cout << "Testing char" << std::endl;
        test_char<char>();
        std::cout << "Testing wchar_t" << std::endl;
        test_char<wchar_t>();
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


