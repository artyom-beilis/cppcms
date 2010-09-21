//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <booster/locale/generator.h>
#include <booster/locale/localization_backend.h>
#include <booster/locale/message.h>
#include <booster/locale/encoding.h>
#include "test_locale.h"
#include "test_locale_tools.h"

namespace bl = booster::locale;

std::string same_s(std::string s)
{
    return s;
}

std::wstring same_w(std::wstring s)
{
    return s;
}

#ifdef BOOSTER_HAS_CHAR16_T
std::u16string same_u16(std::u16string s)
{
    return s;
}
#endif

#ifdef BOOSTER_HAS_CHAR32_T
std::u32string same_u32(std::u32string s)
{
    return s;
}
#endif

template<typename Char>
void strings_equal(std::string c,std::string s,std::string p,int n,std::string iexpected,std::locale const &l,std::string domain)
{
    typedef std::basic_string<Char> string_type;
    string_type expected=to_correct_string<Char>(iexpected,l);
    if(domain=="default") {
        TEST(bl::translate(c,s,p,n).str<Char>(l)==expected);
        char const *c_c_str = c.c_str(),*s_c_str=s.c_str(), *p_c_str=p.c_str(); // workaround gcc-3.4 bug
        TEST(bl::translate(c_c_str,s_c_str,p_c_str,n).str<Char>(l)==expected);
        std::locale tmp_locale=std::locale();
        std::locale::global(l);
        string_type tmp=bl::translate(c,s,p,n);
        TEST(tmp==expected);
        tmp=bl::translate(c,s,p,n).str<Char>();
        TEST(tmp==expected);
        std::locale::global(tmp_locale);

        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::translate(c,s,p,n);
        TEST(ss.str()==expected);
    }
    TEST(bl::translate(c,s,p,n).str<Char>(l,domain)==expected);
    std::locale tmp_locale=std::locale();
    std::locale::global(l);
    TEST(bl::translate(c,s,p,n).str<Char>(domain)==expected);
    std::locale::global(tmp_locale);
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(c,s,p,n);
        TEST(ss.str()==expected);
    }
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(c.c_str(),s.c_str(),p.c_str(),n);
        TEST(ss.str()==expected);
    }
}




template<typename Char>
void strings_equal(std::string s,std::string p,int n,std::string iexpected,std::locale const &l,std::string domain)
{
    typedef std::basic_string<Char> string_type;
    string_type expected=to_correct_string<Char>(iexpected,l);
    if(domain=="default") {
        TEST(bl::translate(s,p,n).str<Char>(l)==expected);
        char const *s_c_str=s.c_str(), *p_c_str=p.c_str(); // workaround gcc-3.4 bug
        TEST(bl::translate(s_c_str,p_c_str,n).str<Char>(l)==expected);
        std::locale tmp_locale=std::locale();
        std::locale::global(l);
        string_type tmp=bl::translate(s,p,n);
        TEST(tmp==expected);
        tmp=bl::translate(s,p,n).str<Char>();
        TEST(tmp==expected);
        std::locale::global(tmp_locale);

        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::translate(s,p,n);
        TEST(ss.str()==expected);
    }
    TEST(bl::translate(s,p,n).str<Char>(l,domain)==expected);
    std::locale tmp_locale=std::locale();
    std::locale::global(l);
    TEST(bl::translate(s,p,n).str<Char>(domain)==expected);
    std::locale::global(tmp_locale);
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(s,p,n);
        TEST(ss.str()==expected);
    }
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(s.c_str(),p.c_str(),n);
        TEST(ss.str()==expected);
    }
}


template<typename Char>
void strings_equal(std::string c,std::string original,std::string iexpected,std::locale const &l,std::string domain)
{
    typedef std::basic_string<Char> string_type;
    string_type expected=to_correct_string<Char>(iexpected,l);
    if(domain=="default") {
        TEST(bl::translate(c,original).str<Char>(l)==expected);
        char const *original_c_str=original.c_str(); // workaround gcc-3.4 bug
        char const *context_c_str = c.c_str();
        TEST(bl::translate(context_c_str,original_c_str).str<Char>(l)==expected);
        std::locale tmp_locale=std::locale();
        std::locale::global(l);
        string_type tmp=bl::translate(c,original);
        TEST(tmp==expected);
        tmp=bl::translate(c,original).str<Char>();
        TEST(tmp==expected);
        std::locale::global(tmp_locale);

        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::translate(c,original);
        TEST(ss.str()==expected);
    }
    TEST(bl::translate(c,original).str<Char>(l,domain)==expected);
    std::locale tmp_locale=std::locale();
    std::locale::global(l);
    TEST(bl::translate(c,original).str<Char>(domain)==expected);
    std::locale::global(tmp_locale);
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(c,original);
        TEST(ss.str()==expected);
    }
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(c.c_str(),original.c_str());
        TEST(ss.str()==expected);
    }
}




template<typename Char>
void strings_equal(std::string original,std::string iexpected,std::locale const &l,std::string domain)
{
    typedef std::basic_string<Char> string_type;
    string_type expected=to_correct_string<Char>(iexpected,l);
    if(domain=="default") {
        TEST(bl::translate(original).str<Char>(l)==expected);
        char const *original_c_str=original.c_str(); // workaround gcc-3.4 bug
        TEST(bl::translate(original_c_str).str<Char>(l)==expected);
        std::locale tmp_locale=std::locale();
        std::locale::global(l);
        string_type tmp=bl::translate(original);
        TEST(tmp==expected);
        tmp=bl::translate(original).str<Char>();
        TEST(tmp==expected);
        std::locale::global(tmp_locale);

        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::translate(original);
        TEST(ss.str()==expected);
    }
    TEST(bl::translate(original).str<Char>(l,domain)==expected);
    std::locale tmp_locale=std::locale();
    std::locale::global(l);
    TEST(bl::translate(original).str<Char>(domain)==expected);
    std::locale::global(tmp_locale);
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(original);
        TEST(ss.str()==expected);
    }
    {
        std::basic_ostringstream<Char> ss;
        ss.imbue(l);
        ss << bl::as::domain(domain) << bl::translate(original.c_str());
        TEST(ss.str()==expected);
    }
}

void test_cntranslate(std::string c,std::string s,std::string p,int n,std::string expected,std::locale const &l,std::string domain)
{
    strings_equal<char>(c,s,p,n,expected,l,domain);
    strings_equal<wchar_t>(c,s,p,n,expected,l,domain);
    #ifdef BOOSTER_HAS_CHAR16_T
    strings_equal<char16_t>(c,s,p,n,expected,l,domain);
    #endif
    #ifdef BOOSTER_HAS_CHAR32_T
    strings_equal<char32_t>(c,s,p,n,expected,l,domain);
    #endif
}


void test_ntranslate(std::string s,std::string p,int n,std::string expected,std::locale const &l,std::string domain)
{
    strings_equal<char>(s,p,n,expected,l,domain);
    strings_equal<wchar_t>(s,p,n,expected,l,domain);
    #ifdef BOOSTER_HAS_CHAR16_T
    strings_equal<char16_t>(s,p,n,expected,l,domain);
    #endif
    #ifdef BOOSTER_HAS_CHAR32_T
    strings_equal<char32_t>(s,p,n,expected,l,domain);
    #endif
}

void test_ctranslate(std::string c,std::string original,std::string expected,std::locale const &l,std::string domain)
{
    strings_equal<char>(c,original,expected,l,domain);
    strings_equal<wchar_t>(c,original,expected,l,domain);
    #ifdef BOOSTER_HAS_CHAR16_T
    strings_equal<char16_t>(c,original,expected,l,domain);
    #endif
    #ifdef BOOSTER_HAS_CHAR32_T
    strings_equal<char32_t>(c,original,expected,l,domain);
    #endif
}



void test_translate(std::string original,std::string expected,std::locale const &l,std::string domain)
{
    strings_equal<char>(original,expected,l,domain);
    strings_equal<wchar_t>(original,expected,l,domain);
    #ifdef BOOSTER_HAS_CHAR16_T
    strings_equal<char16_t>(original,expected,l,domain);
    #endif
    #ifdef BOOSTER_HAS_CHAR32_T
    strings_equal<char32_t>(original,expected,l,domain);
    #endif
}


#ifdef BOOSTER_WIN_NATIVE

void test_wide_path(int argc,char **argv)
{
    std::cout << "Testing loading catalogs from wide path" << std::endl;
    booster::locale::generator g;
    g.add_messages_domain("default");
    if(argc==2)
        g.add_messages_path(booster::locale::conv::to_utf<wchar_t>(argv[1],"windows-1252"));
    else
        g.add_messages_path(L"./");

    std::locale l=g("he_IL.UTF-8");

    TEST(booster::locale::gettext("hello",l)=="שלום");

}

#endif


int main(int argc,char **argv)
{
    try {
        std::string def[] = { "icu" , "posix", "winapi", "std" };
        for(int type = 0 ; type < int(sizeof(def)/sizeof(def[0])) ; type ++ ) {
            booster::locale::localization_backend_manager tmp_backend = booster::locale::localization_backend_manager::global();
            tmp_backend.select(def[type]);
            booster::locale::localization_backend_manager::global(tmp_backend);
            
            std::cout << "Testing for backend --------- " << def[type] << std::endl;

            #ifdef BOOSTER_WIN_NATIVE
            test_wide_path(argc,argv);
            #endif

            booster::locale::generator g;
            g.add_messages_domain("default");
            g.add_messages_domain("simple");
            g.add_messages_domain("full");
            g.add_messages_domain("fall");
            if(argc==2)
                g.add_messages_path(argv[1]);
            else
                g.add_messages_path("./");

            
            std::string locales[] = { "he_IL.UTF-8", "he_IL.ISO8859-8" };

            for(unsigned i=0;i<sizeof(locales)/sizeof(locales[0]);i++){
                std::locale l=g(locales[i]);
                
                std::cout << "Testing "<<locales[i]<<std::endl;
                std::cout << " single forms" << std::endl;

                test_translate("hello","שלום",l,"default");
                test_translate("hello","היי",l,"simple");
                test_translate("hello","hello",l,"undefined");
                test_translate("untranslated","untranslated",l,"default");
                // Check removal of old "context" information
                test_translate("#untranslated","#untranslated",l,"default");
                test_translate("##untranslated","##untranslated",l,"default");
                test_ctranslate("context","hello","שלום בהקשר אחר",l,"default");
                test_translate("#hello","#שלום",l,"default");

                std::cout << " plural forms" << std::endl;

                {
                    test_ntranslate("x day","x days",0,"x ימים",l,"default");
                    test_ntranslate("x day","x days",1,"יום x",l,"default");
                    test_ntranslate("x day","x days",2,"יומיים",l,"default");
                    test_ntranslate("x day","x days",3,"x ימים",l,"default");
                    test_ntranslate("x day","x days",20,"x יום",l,"default");
                    
                    test_ntranslate("x day","x days",0,"x days",l,"undefined");
                    test_ntranslate("x day","x days",1,"x day",l,"undefined");
                    test_ntranslate("x day","x days",2,"x days",l,"undefined");
                    test_ntranslate("x day","x days",20,"x days",l,"undefined");
                }
                std::cout << " plural forms with context" << std::endl;
                {
                    std::string inp = "context"; 
                    std::string out = "בהקשר "; 

                    test_cntranslate(inp,"x day",out+"x days",0,out+"x ימים",l,"default");
                    test_cntranslate(inp,"x day",out+"x days",1,out+"יום x",l,"default");
                    test_cntranslate(inp,"x day",out+"x days",2,out+"יומיים",l,"default");
                    test_cntranslate(inp,"x day",out+"x days",3,out+"x ימים",l,"default");
                    test_cntranslate(inp,"x day",out+"x days",20,out+"x יום",l,"default");
                    
                    test_cntranslate(inp,"x day","x days",0,"x days",l,"undefined");
                    test_cntranslate(inp,"x day","x days",1,"x day",l,"undefined");
                    test_cntranslate(inp,"x day","x days",2,"x days",l,"undefined");
                    test_cntranslate(inp,"x day","x days",20,"x days",l,"undefined");
                }
            }
            std::cout << "Testing fallbacks" <<std::endl;
            test_translate("test","he_IL",g("he_IL.UTF-8"),"full");
            test_translate("test","he",g("he_IL.UTF-8"),"fall");
            
            std::cout << "Testing automatic conversions " << std::endl;
            std::locale::global(g("he_IL.UTF-8"));


            TEST(same_s(bl::translate("hello"))=="שלום");
            TEST(same_w(bl::translate("hello"))==to<wchar_t>("שלום"));
            
            #ifdef BOOSTER_HAS_CHAR16_T
            TEST(same_u16(bl::translate("hello"))==to<char16_t>("שלום"));
            #endif
            
            #ifdef BOOSTER_HAS_CHAR32_T
            TEST(same_u32(bl::translate("hello"))==to<char32_t>("שלום"));
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
