//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_WITH_ICU
#include <iostream>
int main()
{
        std::cout << "ICU is not build... Skipping" << std::endl;
}
#else

#include <booster/locale/generator.h>
#include <booster/locale/info.h>
#include <booster/locale/collator.h>
#include <iomanip>
#include "test_locale.h"


bool has_collator(std::locale const &l)
{
    return std::has_facet<std::collate<char> >(l) 
        && dynamic_cast<booster::locale::collator<char> const *>(&std::use_facet<std::collate<char> >(l));
}

struct test_facet : public std::locale::facet {
    test_facet() : std::locale::facet(0) {}
    static std::locale::id id;
};

std::locale::id test_facet::id;


int main()
{
    try {
        booster::locale::generator g;
        std::locale l=g("en_US.UTF-8");
        TEST(has_collator(l));

        g.categories(g.categories() ^ booster::locale::collation_facet);
        g.locale_cache_enabled(true);
        g("en_US.UTF-8");
        g.categories(g.categories() | booster::locale::collation_facet);
        l=g("en_US.UTF-8");
        TEST(!has_collator(l));
        g.clear_cache();
        g.locale_cache_enabled(false);
        l=g("en_US.UTF-8");
        TEST(has_collator(l));
        g.characters(g.characters() ^ booster::locale::char_facet);
        l=g("en_US.UTF-8");
        TEST(!has_collator(l));
        g.characters(g.characters() | booster::locale::char_facet);
        l=g("en_US.UTF-8");
        TEST(has_collator(l));

        l=g("en_US.ISO8859-1");
        TEST(std::use_facet<booster::locale::info>(l).language()=="en");
        TEST(std::use_facet<booster::locale::info>(l).country()=="US");
        TEST(!std::use_facet<booster::locale::info>(l).utf8());
        TEST(std::use_facet<booster::locale::info>(l).encoding()=="iso8859-1");

        l=g("en_US.UTF-8");
        TEST(std::use_facet<booster::locale::info>(l).language()=="en");
        TEST(std::use_facet<booster::locale::info>(l).country()=="US");
        TEST(std::use_facet<booster::locale::info>(l).utf8());

        l=g("en_US.ISO8859-1");
        TEST(std::use_facet<booster::locale::info>(l).language()=="en");
        TEST(std::use_facet<booster::locale::info>(l).country()=="US");
        TEST(!std::use_facet<booster::locale::info>(l).utf8());
        TEST(std::use_facet<booster::locale::info>(l).encoding()=="iso8859-1");

        l=g("en_US.ISO8859-1");
        TEST(std::use_facet<booster::locale::info>(l).language()=="en");
        TEST(std::use_facet<booster::locale::info>(l).country()=="US");
        TEST(!std::use_facet<booster::locale::info>(l).utf8());
        TEST(std::use_facet<booster::locale::info>(l).encoding()=="iso8859-1");

        std::locale l_wt(std::locale::classic(),new test_facet);
        
        TEST(std::has_facet<test_facet>(g.generate(l_wt,"en_US.UTF-8")));
        TEST(std::has_facet<test_facet>(g.generate(l_wt,"en_US.ISO8859-1")));
        TEST(!std::has_facet<test_facet>(g("en_US.UTF-8")));
        TEST(!std::has_facet<test_facet>(g("en_US.ISO8859-1")));

        g.locale_cache_enabled(true);
        g.generate(l_wt,"en_US.UTF-8");
        g.generate(l_wt,"en_US.ISO8859-1");
        TEST(std::has_facet<test_facet>(g("en_US.UTF-8")));
        TEST(std::has_facet<test_facet>(g("en_US.ISO8859-1")));
        TEST(std::use_facet<booster::locale::info>(g("en_US.UTF-8")).utf8());
        TEST(!std::use_facet<booster::locale::info>(g("en_US.ISO8859-1")).utf8());

    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
#endif // NOICU
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
