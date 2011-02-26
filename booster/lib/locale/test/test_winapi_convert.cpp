//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
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
#include <iostream>

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

    std::cout << "- Testing " << name << std::endl;
    l=gen(name);
    test_one<CharType>(l,"Façade","façade","FAÇADE");
    
    
    name = "tr_TR.UTF-8";
    std::cout << "Testing " << name << std::endl;
    test_one<CharType>(gen(name),"i","i","İ");

}

template<typename Char>
void test_normc(std::basic_string<Char> orig,std::basic_string<Char> normal,booster::locale::norm_type type)
{
    std::locale l = booster::locale::generator().generate("en_US.UTF-8");
    TEST(booster::locale::normalize(orig,type,l)==normal);
    TEST(booster::locale::normalize(orig.c_str(),type,l)==normal);
    TEST(booster::locale::normalize(orig.c_str(),orig.c_str()+orig.size(),type,l)==normal);
}


void test_norm(std::string orig,std::string normal,booster::locale::norm_type type)
{
    test_normc<char>(orig,normal,type);
    test_normc<wchar_t>(to<wchar_t>(orig),to<wchar_t>(normal),type);
}

int main()
{
    try {
        booster::locale::localization_backend_manager mgr = booster::locale::localization_backend_manager::global();
        mgr.select("winapi");
        booster::locale::localization_backend_manager::global(mgr);

        std::cout << "Testing char" << std::endl;
        test_char<char>();
        std::cout << "Testing wchar_t" << std::endl;
        test_char<wchar_t>();
        
        std::cout << "Testing Unicode normalization" << std::endl;
        test_norm("\xEF\xAC\x81","\xEF\xAC\x81",booster::locale::norm_nfd); /// ligature fi
        test_norm("\xEF\xAC\x81","\xEF\xAC\x81",booster::locale::norm_nfc);
		#if defined(_WIN32_NT) && _WIN32_NT >= 0x600
        test_norm("\xEF\xAC\x81","fi",booster::locale::norm_nfkd);
        test_norm("\xEF\xAC\x81","fi",booster::locale::norm_nfkc);
		#endif
        test_norm("ä","ä",booster::locale::norm_nfd); // ä to a and accent
        test_norm("ä","ä",booster::locale::norm_nfc);
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


