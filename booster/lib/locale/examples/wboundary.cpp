//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

//
// ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
//
// BIG FAT WARNING FOR Microsoft Visual Studio Users
//
// YOU NEED TO CONVERT THIS SOURCE FILE ENCODING TO UTF-8 WITH BOM ENCODING.
//
// Unfortunately MSVC understands that the source code is encoded as
// UTF-8 only if you add useless BOM in the beginning.
//
// So, before you compile "wide" examples with MSVC, please convert them to text
// files with BOM. There are two very simple ways to do it:
//
// 1. Open file with Notepad and save it from there. It would convert 
//    it to file with BOM.
// 2. In Visual Studio go File->Advances Save Options... and select
//    Unicode (UTF-8  with signature) Codepage 65001
//
// Note: once converted to UTF-8 with BOM, this source code would not
// compile with other compilers, because no-one uses BOM with UTF-8 today
// because it is absolutely meaningless in context of UTF-8.
//
// ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
//
#include <booster/locale.h>
#include <iostream>
#include <cassert>
#include <ctime>

#ifndef BOOSTER_NO_SWPRINTF    
int main(int argc,char **argv)
{
    using namespace booster::locale;
    using namespace std;

    std::ios_base::sync_with_stdio(false);

    generator gen;
    locale::global(locale(""));
    locale loc;
    if(argc == 1)
        loc=gen(""); 
    else if(argc == 2)
        loc=gen(argv[1]);
    else
        loc=gen(argv[1],argv[2]);

    // Create system default locale

    locale::global(loc); 
    // Make it system global
    
    wcout.imbue(loc);
    // Set as default locale for output

    wstring text=L"Hello World! あにま! Linux2.6 and Windows7 is word and number. שָלוֹם עוֹלָם!";

    wcout<<text<<endl;

    typedef boundary::token_iterator<std::wstring::iterator> iter_type;
    typedef boundary::mapping<iter_type>  mapping_type;
    mapping_type index(boundary::word,text.begin(),text.end());
    iter_type p,e;

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"Part ["<<*p<<L"] has ";
        if(p.mark() & boundary::word_number)
            wcout<<L"number ";
        if(p.mark() & boundary::word_letter)
            wcout<<L"letter ";
        if(p.mark() & boundary::word_kana)
            wcout<<L"kana characters ";
        if(p.mark() & boundary::word_ideo)
            wcout<<L"ideographic characters ";
        wcout<<endl;
    }

    index.map(boundary::character,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"|" <<*p ;
    }
    wcout<<L"|\n\n";

    index.map(boundary::line,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"|" <<*p ;
    }
    wcout<<L"|\n\n";

    index.map(boundary::sentence,text.begin(),text.end());

    for(p=index.begin(),e=index.end();p!=e;++p) {
        wcout<<L"|" <<*p ;
    }
    wcout<<"|\n\n";
    
}
#else
int main()
{
    std::cout<<"This platform does not support wcout"<<std::endl;
}
#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
