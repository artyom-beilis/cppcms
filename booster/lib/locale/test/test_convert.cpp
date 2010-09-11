//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/locale/conversion.h>
#include <booster/locale/generator.h>
#include <booster/locale/info.h>
#include <iomanip>
#include "test_locale.h"


template<typename Char>
void test_normc(std::basic_string<Char> orig,std::basic_string<Char> normal,booster::locale::norm_type type)
{
    std::locale l = booster::locale::generator().generate("en_US.UTF-8");
    TEST(normalize(orig,type,l)==normal);
    TEST(normalize(orig.c_str(),type,l)==normal);
    TEST(normalize(orig.c_str(),orig.c_str()+orig.size(),type,l)==normal);
}

void test_norm(std::string orig,std::string normal,booster::locale::norm_type type)
{
    test_normc<char>(orig,normal,type);
    test_normc<wchar_t>(to<wchar_t>(orig),to<wchar_t>(normal),type);
    #ifdef BOOSTER_HAS_CHAR16_T
    test_normc<char16_t>(to<char16_t>(orig),to<char16_t>(normal),type);
    #endif
    #ifdef BOOSTER_HAS_CHAR32_T
    test_normc<char32_t>(to<char32_t>(orig),to<char32_t>(normal),type);
    #endif
}

#define TEST_A(Chr,how,source,dest)                                                            \
    do {                                                                                    \
        booster::locale::info const &inf=std::use_facet<booster::locale::info>(std::locale());    \
        std::cout <<"Testing " #how " for " #Chr ", lang="<<inf.language();                    \
        if(std::string("char")==#Chr) std::cout <<" charset="<<    inf.encoding();                \
        std::cout << std::endl;                                                                \
        std::basic_string<Chr> source_s=(source),dest_s=(dest);                                \
        TEST(booster::locale::how(source_s)==dest_s);                                            \
        TEST(booster::locale::how(source_s.c_str())==dest_s);                                    \
        TEST(booster::locale::how(source_s.c_str(),source_s.c_str()+source_s.size())==dest_s);\
    }while(0)

#define TEST_ALL_CASES                                        \
        do {                                                \
            eight_bit=true;                                    \
            std::locale::global(gen("en_US.UTF-8"));        \
            TEST_V(to_upper,"grüßen i","GRÜSSEN I");        \
            TEST_V(to_lower,"Façade","façade");                \
            TEST_V(to_title,"façadE world","Façade World");    \
            TEST_V(fold_case,"Hello World","hello world");    \
            std::locale::global(gen("tr_TR.UTF-8"));        \
            eight_bit=false;                                \
            TEST_V(to_upper,"i","İ");                        \
            TEST_V(to_lower,"İ","i");                        \
        }while(0)
    
    
int main()
{
    try {
        {
            using namespace booster::locale;
            std::cout << "Testing Unicode normalization" << std::endl;
            test_norm("\xEF\xAC\x81","\xEF\xAC\x81",norm_nfd); /// ligature fi
            test_norm("\xEF\xAC\x81","\xEF\xAC\x81",norm_nfc);
            test_norm("\xEF\xAC\x81","fi",norm_nfkd);
            test_norm("\xEF\xAC\x81","fi",norm_nfkc);
            test_norm("ä","ä",norm_nfd); // ä to a and accent
            test_norm("ä","ä",norm_nfc);
        }

        booster::locale::generator gen;
        bool eight_bit=true;
        
        #define TEST_V(how,source_s,dest_s)                                    \
        do {                                                                \
            TEST_A(char,how,source_s,dest_s);                                \
            if(eight_bit) {                                                    \
                std::locale tmp=std::locale();                                \
                std::locale::global(gen("en_US.ISO8859-1"));                \
                TEST_A(char,how,to<char>(source_s),to<char>(dest_s));        \
                std::locale::global(tmp);                                    \
            }                                                                \
        }while(0)
        
        TEST_ALL_CASES;
        #undef TEST_V

        #define TEST_V(how,source_s,dest_s) TEST_A(wchar_t,how,to<wchar_t>(source_s),to<wchar_t>(dest_s))
        TEST_ALL_CASES;
        #undef TEST_V

        #ifdef BOOSTER_HAS_CHAR16_T
        #define TEST_V(how,source_s,dest_s) TEST_A(char16_t,how,to<char16_t>(source_s),to<char16_t>(dest_s))
        TEST_ALL_CASES;
        #undef TEST_V
        #endif

        #ifdef BOOSTER_HAS_CHAR32_T
        #define TEST_V(how,source_s,dest_s) TEST_A(char32_t,how,to<char32_t>(source_s),to<char32_t>(dest_s))
        TEST_ALL_CASES;
        #undef TEST_V
        #endif
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


