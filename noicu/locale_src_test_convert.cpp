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
        cppcms::locale::info const &inf=std::use_facet<cppcms::locale::info>(std::locale());    \
        std::cout <<"Testing " #how " for lang="<<inf.language();                    	        \
        std::cout <<" charset="<<    inf.encoding();                							\
        std::cout << std::endl;                                                              	\
        std::string source_s=(source),dest_s=(dest);                                            \
        TEST(cppcms::locale::how(source_s)==dest_s);                                            \
        TEST(cppcms::locale::how(source_s.c_str())==dest_s);                                    \
        TEST(cppcms::locale::how(source_s.c_str(),source_s.c_str()+source_s.size())==dest_s);	\
    }while(0)

    
    
int main()
{
    try {
        cppcms::locale::generator gen;
        std::locale::global(gen("en_US.UTF-8"));
        TEST_A(to_lower,"Hello World","hello world");
        TEST_A(to_upper,"Hello World","HELLO WORLD");
        if(std::has_facet<std::ctype<wchar_t> >(std::locale())) {
            TEST_A(to_lower,"АРТЁМ","артём");
            TEST_A(to_upper,"Артём","АРТЁМ");
        }
        std::locale::global(gen("tr_TR.UTF-8"));
        if( std::locale().name()=="tr_TR.UTF-8" 
            && std::has_facet<std::ctype<wchar_t> >(std::locale())) 
        {
            TEST_A(to_upper,"i","İ");                        
            TEST_A(to_lower,"İ","i");                        
        }
        std::locale::global(gen("en_US.ISO-8859-1"));
        if(std::locale().name() == "en_US.ISO-8859-1") {
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


