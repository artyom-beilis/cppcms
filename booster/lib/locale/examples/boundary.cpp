//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/locale.h>
#include <iostream>
#include <cassert>
#include <ctime>

int main()
{
    using namespace booster::locale;
    using namespace std;

    generator gen;
    // Make system default locale global
    std::locale loc = gen("");
    locale::global(loc); 
    cout.imbue(loc);
    

    string text="Hello World! あにま! Linux2.6 and Windows7 is word and number. שָלוֹם עוֹלָם!";

    cout<<text<<endl;

    typedef boundary::token_iterator<std::string::iterator> iter_type;
    typedef boundary::mapping<iter_type>  mapping_type;
    mapping_type index(boundary::word,text.begin(),text.end());
    iter_type p,e;

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout<<"Part ["<<*p<<"] has ";
        if(p.mark() & boundary::word_number)
            cout<<"number ";
        if(p.mark() & boundary::word_letter)
            cout<<"letter ";
        if(p.mark() & boundary::word_kana)
            cout<<"kana characters ";
        if(p.mark() & boundary::word_ideo)
            cout<<"ideographic characters ";
        cout<<endl;
    }

    index.map(boundary::character,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout<<"|" <<*p ;
    }
    cout<<"|\n\n";

    index.map(boundary::line,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout<<"|" <<*p ;
    }
    cout<<"|\n\n";

    index.map(boundary::sentence,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        cout<<"|" <<*p ;
    }
    cout<<"|\n\n";
    
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
