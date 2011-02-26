//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include "lcid.h"
#include <string.h>
#include <string>
#include <sstream>
#include <map>

#include "../util/locale_data.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <booster/thread.h>

namespace booster {
namespace locale {
namespace impl_win {

static volatile bool table_is_ready;

booster::mutex &lcid_table_mutex()
{
    static booster::mutex m;
    return m;
}

std::map<std::string,unsigned> &real_lcid_table()
{
    static std::map<std::string,unsigned> table;
    return table;
}

BOOL CALLBACK proc(char *s)
{
    std::map<std::string,unsigned> &tbl = real_lcid_table();
    try {
        std::istringstream ss;
        ss.str(s);
        ss >> std::hex;

        unsigned lcid ;
        ss >>lcid;

        char iso_639_lang[16];
        char iso_3166_country[16];
        if(GetLocaleInfoA(lcid,LOCALE_SISO639LANGNAME,iso_639_lang,sizeof(iso_639_lang))==0)
            return FALSE;
        std::string lc_name = iso_639_lang;
        if(GetLocaleInfoA(lcid,LOCALE_SISO3166CTRYNAME,iso_3166_country,sizeof(iso_3166_country))!=0) {
            lc_name += "_";
            lc_name += iso_3166_country;
        }
        std::map<std::string,unsigned>::iterator p = tbl.find(lc_name);
        if(p!=tbl.end()) {
            if(p->second > lcid)
                p->second = lcid;
        }
        else {
            tbl[lc_name]=lcid;
        }
    }
    catch(...) {
        tbl.clear();
        return FALSE;
    }
    return TRUE;
}


std::map<std::string,unsigned>  const &get_ready_lcid_table()
{
    if(table_is_ready)
        return real_lcid_table();
    else {
        booster::unique_lock<booster::mutex> lock(lcid_table_mutex());
        std::map<std::string,unsigned> &table = real_lcid_table();
        if(!table.empty())
            return table;
        EnumSystemLocalesA(proc,LCID_INSTALLED);
        table_is_ready = true;
        return table;
    }
}

unsigned locale_to_lcid(std::string const &locale_name)
{
    if(locale_name.empty()) {
        return LOCALE_USER_DEFAULT;
    } 
    booster::locale::util::locale_data d;
    d.parse(locale_name);
    std::string id = d.language;

    if(!d.country.empty()) {
        id+="_"+d.country;
    }
    if(!d.variant.empty()) {
        id+="@" + d.variant;
    }

    std::map<std::string,unsigned> const &tbl = get_ready_lcid_table();
    std::map<std::string,unsigned>::const_iterator p = tbl.find(id);
    
    unsigned lcid = 0;
    if(p!=tbl.end())
        lcid = p->second;
    return lcid;
}


} // impl_win
} // locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
