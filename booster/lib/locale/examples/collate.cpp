//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <string>
#include <set>

#include <booster/locale.h>

using namespace std;
using namespace booster::locale;

int main(int argc,char **argv)
{
     if(argc!=3) {
          cerr<<"Usage collate Locale-ID Encoding"<<endl;
          return 1;
     }
     generator gen;

     /// Set global locale to requested
     std::locale::global(gen.generate(argv[1],argv[2]));

     /// Create a set that includes all strings sorted with primary collation level
     typedef std::set<std::string,comparator<char> > set_type;
     set_type all_strings;

     /// Read all strings into the set
     while(!cin.eof()) {
          std::string tmp;
          getline(cin,tmp);
          all_strings.insert(tmp);
     }
     /// Print them out
     for(set_type::iterator p=all_strings.begin();p!=all_strings.end();++p) {
          cout<<*p<<endl;
     }

}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
