//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/locale/encoding.h>
#include <booster/locale/generator.h>
#include <booster/locale/localization_backend.h>
#include <booster/locale/info.h>
#include <fstream>
#include "test_locale.h"
#include "test_locale_tools.h"

bool test_iso;
bool test_utf;
bool test_sjis;

std::string he_il_8bit;
std::string en_us_8bit;
std::string ja_jp_shiftjis;


template<typename Char>
std::basic_string<Char> read_file(std::basic_istream<Char> &in)
{
    std::basic_string<Char> res;
    Char c;
    while(in.get(c))
        res+=c;
    return res;
}


template<typename Char>
void test_ok(std::string file,std::locale const &l,std::basic_string<Char> cmp=std::basic_string<Char>())
{
    if(cmp.empty())
        cmp=to<Char>(file);
    std::ofstream test("testi.txt");
    test << file;
    test.close();
    typedef std::basic_fstream<Char> stream_type;

    stream_type f1("testi.txt",stream_type::in);
    f1.imbue(l);
    TEST(read_file<Char>(f1) == cmp); 
    f1.close();

    stream_type f2("testo.txt",stream_type::out);
    f2.imbue(l);
    f2 << cmp;
    f2.close();

    std::ifstream testo("testo.txt");
    TEST(read_file<char>(testo) == file);
}

template<typename Char>
void test_rfail(std::string file,std::locale const &l,int pos)
{
    std::ofstream test("testi.txt");
    test << file;
    test.close();
    typedef std::basic_fstream<Char> stream_type;

    stream_type f1("testi.txt",stream_type::in);
    f1.imbue(l);
    Char c;
    for(int i=0;i<pos;i++)
        TEST(f1.get(c));
    TEST(f1.get(c).fail());
}

template<typename Char>
void test_wfail(std::string file,std::locale const &l,int pos)
{
    typedef std::basic_fstream<Char> stream_type;
    stream_type f1("testo.txt",stream_type::out);
    f1.imbue(l);
    std::basic_string<Char> out=to<Char>(file);
    int i;
    for(i=0;i<pos;i++) {
        f1 << out.at(i);
        f1<<std::flush;
        TEST(f1.good());
    }
    f1 << out.at(i);
    TEST(f1.fail() || (f1<<std::flush).fail());
}


template<typename Char>
void test_for_char()
{
    booster::locale::generator g;
    if(test_utf) {
        std::cout << "    UTF-8" << std::endl;
        test_ok<Char>("grüße\nn i",g("en_US.UTF-8"));
        test_rfail<Char>("abc\xFF\xFF",g("en_US.UTF-8"),3);
        std::cout << "    Testing codepoints above 0xFFFF" << std::endl;
        std::cout << "      Single U+2008A" << std::endl;
        test_ok<Char>("\xf0\xa0\x82\x8a",g("en_US.UTF-8")); // U+2008A
        std::cout << "      Single U+2008A withing text" << std::endl;
        test_ok<Char>("abc\"\xf0\xa0\x82\x8a\"",g("en_US.UTF-8")); // U+2008A
        std::string one = "\xf0\xa0\x82\x8a";
        std::string res;
        for(unsigned i=0;i<1000;i++)
            res+=one;
        std::cout << "      U+2008A x 1000" << std::endl;
        test_ok<Char>(res.c_str(),g("en_US.UTF-8")); // U+2008A
    }
    else {
        std::cout << "    UTF-8 Not supported " << std::endl;
    }
    
    if(test_iso) {
        std::cout << "    ISO8859-8" << std::endl;
        test_ok<Char>("hello \xf9\xec\xe5\xed",g(he_il_8bit),to<Char>("hello שלום"));
        std::cout << "    ISO8859-1" << std::endl;
        test_ok<Char>(to<char>("grüße\nn i"),g(en_us_8bit),to<Char>("grüße\nn i"));
        test_wfail<Char>("grüßen שלום",g(en_us_8bit),7);
    }

    if(test_sjis) {
        std::cout << "    Shift-JIS" << std::endl;
        test_ok<Char>("\x93\xfa\x96\x7b",g(ja_jp_shiftjis),
                booster::locale::conv::to_utf<Char>("\xe6\x97\xa5\xe6\x9c\xac","UTF-8"));  // Japan
    }
}
void test_wide_io()
{
    std::cout << "  wchar_t" << std::endl;
    test_for_char<wchar_t>();
    
    #if defined BOOSTER_HAS_CHAR16_T && !defined(BOOSTER_NO_CHAR16_T_CODECVT)
    std::cout << "  char16_t" << std::endl;
    test_for_char<char16_t>();
    #endif
    #if defined BOOSTER_HAS_CHAR32_T && !defined(BOOSTER_NO_CHAR32_T_CODECVT)
    std::cout << "  char32_t" << std::endl;
    test_for_char<char32_t>();
    #endif
}

template<typename Char>
void test_pos(std::string source,std::basic_string<Char> target,std::string encoding)
{
    using namespace booster::locale::conv;
    booster::locale::generator g;
    std::locale l= encoding == "ISO8859-8" ? g("he_IL."+encoding) : g("en_US."+encoding);
    TEST(to_utf<Char>(source,encoding)==target);
    TEST(to_utf<Char>(source.c_str(),encoding)==target);
    TEST(to_utf<Char>(source.c_str(),source.c_str()+source.size(),encoding)==target);
    
    TEST(to_utf<Char>(source,l)==target);
    TEST(to_utf<Char>(source.c_str(),l)==target);
    TEST(to_utf<Char>(source.c_str(),source.c_str()+source.size(),l)==target);

    TEST(from_utf<Char>(target,encoding)==source);
    TEST(from_utf<Char>(target.c_str(),encoding)==source);
    TEST(from_utf<Char>(target.c_str(),target.c_str()+target.size(),encoding)==source);
    
    TEST(from_utf<Char>(target,l)==source);
    TEST(from_utf<Char>(target.c_str(),l)==source);
    TEST(from_utf<Char>(target.c_str(),target.c_str()+target.size(),l)==source);
}

#define TESTF(X) TEST_THROWS(X,booster::locale::conv::conversion_error)

template<typename Char>
void test_to_neg(std::string source,std::basic_string<Char> target,std::string encoding)
{
    using namespace booster::locale::conv;
    booster::locale::generator g;
    std::locale l=g("en_US."+encoding);

    TEST(to_utf<Char>(source,encoding)==target);
    TEST(to_utf<Char>(source.c_str(),encoding)==target);
    TEST(to_utf<Char>(source.c_str(),source.c_str()+source.size(),encoding)==target);
    TEST(to_utf<Char>(source,l)==target);
    TEST(to_utf<Char>(source.c_str(),l)==target);
    TEST(to_utf<Char>(source.c_str(),source.c_str()+source.size(),l)==target);

    TESTF(to_utf<Char>(source,encoding,stop));
    TESTF(to_utf<Char>(source.c_str(),encoding,stop));
    TESTF(to_utf<Char>(source.c_str(),source.c_str()+source.size(),encoding,stop));
    TESTF(to_utf<Char>(source,l,stop));
    TESTF(to_utf<Char>(source.c_str(),l,stop));
    TESTF(to_utf<Char>(source.c_str(),source.c_str()+source.size(),l,stop));
}

template<typename Char>
void test_from_neg(std::basic_string<Char> source,std::string target,std::string encoding)
{
    using namespace booster::locale::conv;
    booster::locale::generator g;
    std::locale l=g("en_US."+encoding);

    TEST(from_utf<Char>(source,encoding)==target);
    TEST(from_utf<Char>(source.c_str(),encoding)==target);
    TEST(from_utf<Char>(source.c_str(),source.c_str()+source.size(),encoding)==target);
    TEST(from_utf<Char>(source,l)==target);
    TEST(from_utf<Char>(source.c_str(),l)==target);
    TEST(from_utf<Char>(source.c_str(),source.c_str()+source.size(),l)==target);

    TESTF(from_utf<Char>(source,encoding,stop));
    TESTF(from_utf<Char>(source.c_str(),encoding,stop));
    TESTF(from_utf<Char>(source.c_str(),source.c_str()+source.size(),encoding,stop));
    TESTF(from_utf<Char>(source,l,stop));
    TESTF(from_utf<Char>(source.c_str(),l,stop));
    TESTF(from_utf<Char>(source.c_str(),source.c_str()+source.size(),l,stop));
}

template<typename Char>
std::basic_string<Char> utf(char const *s)
{
    return to<Char>(s);
}

template<>
std::basic_string<char> utf(char const *s)
{
    return s;
}

template<typename Char>
void test_with_0()
{
    std::string a("abc\0\0 yz\0",3+2+3+1);
    TEST(booster::locale::conv::from_utf<Char>(booster::locale::conv::to_utf<Char>(a,"UTF-8"),"UTF-8") == a);
    TEST(booster::locale::conv::from_utf<Char>(booster::locale::conv::to_utf<Char>(a,"ISO8859-1"),"ISO8859-1") == a);
}


template<typename Char>
void test_to()
{
    test_pos<Char>(to<char>("grüßen"),utf<Char>("grüßen"),"ISO8859-1");
    test_pos<Char>("\xf9\xec\xe5\xed",utf<Char>("שלום"),"ISO8859-8");
    test_pos<Char>("grüßen",utf<Char>("grüßen"),"UTF-8");
    test_pos<Char>("abc\"\xf0\xa0\x82\x8a\"",utf<Char>("abc\"\xf0\xa0\x82\x8a\""),"UTF-8");
    
    test_to_neg<Char>("g\xFFrüßen",utf<Char>("grüßen"),"UTF-8");
    test_from_neg<Char>(utf<Char>("hello שלום"),"hello ","ISO8859-1");
 
    test_with_0<Char>();
}


int main()
{
    try {
        std::string def[2] = { "icu" , "std" };
        #ifdef BOOSTER_LOCALE_WITH_ICU
        int start = 0;
        #else
        int start = 1;
        #endif
        
        for(int type = start ; type < 2; type ++ ) {
            booster::locale::localization_backend_manager tmp_backend = booster::locale::localization_backend_manager::global();
            tmp_backend.select(def[type]);
            booster::locale::localization_backend_manager::global(tmp_backend);
            
            if(def[type]=="std") {
                en_us_8bit = get_std_name("en_US.ISO8859-1");
                he_il_8bit = get_std_name("he_IL.ISO8859-8");
                ja_jp_shiftjis = get_std_name("ja_JP.SJIS");
            }
            else {
                en_us_8bit = "en_US.ISO8859-1";
                he_il_8bit = "he_IL.ISO8859-8";
                ja_jp_shiftjis = "ja_JP.SJIS";
            }

            std::cout << "Testing for backend " << def[type] << std::endl;

            test_iso = true;
            if(def[type]=="std" && (he_il_8bit.empty() || en_us_8bit.empty())) {
                std::cout << "no iso locales availible, passing" << std::endl;
                test_iso = false;
            }
            test_sjis = true;
            if(def[type]=="std" && ja_jp_shiftjis.empty()) {
                std::cout << "no ShiftJIS locales availible, passing" << std::endl;
                test_sjis = false;
            }

            test_utf = def[type]!="std" || (!get_std_name("en_US.UTF-8").empty() && !get_std_name("he_IL.UTF-8").empty());
            
            std::cout << "Testing wide I/O" << std::endl;
            test_wide_io();
            std::cout << "Testing charset to/from UTF conversion functions" << std::endl;
            std::cout << "  char" << std::endl;
            test_to<char>();
            std::cout << "  wchar_t" << std::endl;
            test_to<wchar_t>();
            #ifdef BOOSTER_HAS_CHAR16_T
            std::cout << "  char16_t" << std::endl;
            test_to<char16_t>();
            #endif
            #ifdef BOOSTER_HAS_CHAR32_T
            std::cout << "  char32_t" << std::endl;
            test_to<char32_t>();
            #endif
        }
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
