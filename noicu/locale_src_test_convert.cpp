//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "locale_conversion.h"
#include "locale_generator.h"
#include "locale_info.h"
#include <iomanip>
#include "locale_src_test_locale.hpp"


#define TEST_A(how,source,dest)                                                            		\
    do {                                                                                    	\
        std::string source_s=(source),dest_s=(dest);                                            \
        TEST(cppcms::locale::how(source_s)==dest_s);                                            \
        TEST(cppcms::locale::how(source_s.c_str())==dest_s);                                    \
        TEST(cppcms::locale::how(source_s.c_str(),source_s.c_str()+source_s.size())==dest_s);	\
    }while(0)

bool has_locale(std::string l,bool wide=false)
{
    bool res;
    try {
        std::locale tmp(l.c_str());
        res = !wide || std::has_facet<std::ctype<wchar_t> >(tmp);
    }
    catch(std::exception const &e)
    {
        res = false;
    }
    std::cout << "Standard library " 
        << (res ? "supports " : "does not support ") 
        << (wide ? "wide " : "") 
        << l << std::endl;
    return res;
}
    
int main()
{
    try {
        cppcms::locale::generator gen;
        
        std::locale::global(gen("en_US.UTF-8"));
        
        std::cout << "Testing ASCII case conversion" << std::endl;
        TEST_A(to_lower,"Hello World","hello world");
        TEST_A(to_upper,"Hello World","HELLO WORLD");

        if(has_locale("en_US.UTF-8",true)) {
            std::cout << "  Testing UTF-8 case conversion " << std::endl;
            TEST_A(to_lower,"АРТЁМ","артём");
            TEST_A(to_upper,"Артём","АРТЁМ");
        }
        
        if(has_locale("tr_TR.UTF-8",true)) {
            std::cout << "  Testing locale dependent case conversion " << std::endl;
            std::locale::global(gen("tr_TR.UTF-8"));
            TEST_A(to_upper,"i","İ");                        
            TEST_A(to_lower,"İ","i");                        
        }

        if(has_locale("en_US.iso88591")) {
            std::cout << "  Testing 8 bits encoding case conversion " << std::endl;
            std::locale::global(gen("en_US.iso88591"));
            TEST_A(to_lower,to<char>("GRÜßEN I"),to<char>("grüßen i"));
            TEST_A(to_upper,to<char>("grüßen i"),to<char>("GRÜßEN I"));
        }
    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4


