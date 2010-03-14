//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "locale_generator.h"
#include "locale_info.h"
#include "locale_collator.h"
#include <iomanip>
#include "locale_src_test_locale.hpp"


bool has_collator(std::locale const &l)
{
    return std::has_facet<std::collate<char> >(l) 
        && dynamic_cast<cppcms::locale::collator<char> const *>(&std::use_facet<std::collate<char> >(l));
}

struct test_facet : public std::locale::facet {
    test_facet() : std::locale::facet(0) {}
    static std::locale::id id;
};

std::locale::id test_facet::id;


int main()
{
    try {
        cppcms::locale::generator g;
        std::locale l=g("en_US.UTF-8");
        TEST(has_collator(l));

        g.categories(g.categories() ^ cppcms::locale::collation_facet);
        g.preload("en_US.UTF-8");
        g.categories(g.categories() | cppcms::locale::collation_facet);
        l=g("en_US.UTF-8");
        TEST(!has_collator(l));
        g.clear_cache();
        l=g("en_US.UTF-8");
        TEST(has_collator(l));
        g.characters(g.characters() ^ cppcms::locale::char_facet);
        l=g("en_US.UTF-8");
        TEST(!has_collator(l));
        g.characters(g.characters() | cppcms::locale::char_facet);
        l=g("en_US.UTF-8");
        TEST(has_collator(l));

        l=g("en_US.ISO-8859-1");
        TEST(std::use_facet<cppcms::locale::info>(l).language()=="en");
        TEST(std::use_facet<cppcms::locale::info>(l).country()=="US");
        TEST(!std::use_facet<cppcms::locale::info>(l).utf8());
        TEST(std::use_facet<cppcms::locale::info>(l).encoding()=="ISO-8859-1");

        g.octet_encoding("UTF-8");
        l=g("en_US");
        TEST(std::use_facet<cppcms::locale::info>(l).language()=="en");
        TEST(std::use_facet<cppcms::locale::info>(l).country()=="US");
        TEST(std::use_facet<cppcms::locale::info>(l).utf8());

        l=g("en_US.ISO-8859-1");
        TEST(std::use_facet<cppcms::locale::info>(l).language()=="en");
        TEST(std::use_facet<cppcms::locale::info>(l).country()=="US");
        TEST(!std::use_facet<cppcms::locale::info>(l).utf8());
        TEST(std::use_facet<cppcms::locale::info>(l).encoding()=="ISO-8859-1");

        l=g("en_US","ISO-8859-1");
        TEST(std::use_facet<cppcms::locale::info>(l).language()=="en");
        TEST(std::use_facet<cppcms::locale::info>(l).country()=="US");
        TEST(!std::use_facet<cppcms::locale::info>(l).utf8());
        TEST(std::use_facet<cppcms::locale::info>(l).encoding()=="ISO-8859-1");

        std::locale l_wt(std::locale::classic(),new test_facet);
        
        TEST(std::has_facet<test_facet>(g.generate(l_wt,"en_US")));
        TEST(std::has_facet<test_facet>(g.generate(l_wt,"en_US","ISO-8859-1")));
        TEST(!std::has_facet<test_facet>(g("en_US")));
        TEST(!std::has_facet<test_facet>(g("en_US","ISO-8859-1")));

        g.preload(l_wt,"en_US");
        g.preload(l_wt,"en_US","ISO-8859-1");
        TEST(std::has_facet<test_facet>(g("en_US")));
        TEST(std::has_facet<test_facet>(g("en_US","ISO-8859-1")));
        TEST(std::use_facet<cppcms::locale::info>(g("en_US")).utf8());
        TEST(!std::use_facet<cppcms::locale::info>(g("en_US","ISO-8859-1")).utf8());

    }
    catch(std::exception const &e) {
        std::cerr << "Failed " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    FINALIZE();

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
