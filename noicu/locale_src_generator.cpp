//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "locale_generator.h"
#include "locale_numeric.h"
#include "locale_info.h"
#include "locale_message.h"
#include <booster/regex.h>

#include <iostream>
namespace cppcms {
    namespace locale {

        namespace {
            template<typename Facet>
            std::locale add_facet(std::locale const &source,Facet *f)
            {
                std::locale tmp(std::locale::classic(),f);
                return source.combine<Facet>(tmp);
            }
        }

        namespace impl {

            class numpunct_utf8 : public std::numpunct_byname<char> {
            public:
                numpunct_utf8(char const *name) :
                    std::numpunct_byname<char>(name)
                {
                }
            protected:
                virtual std::string do_grouping() const
                {
                    return std::string();
                }
            };

            template<bool Intl>
            class moneypunct_utf8 : public std::moneypunct_byname<char,Intl> {
            public:
                moneypunct_utf8(char const *name) :
                    std::moneypunct_byname<char,Intl>(name)
                {
                }
            protected:
                virtual std::string do_grouping() const
                {
                    return std::string();
                }
            };

            std::locale workaround_utf8_bug(std::locale l)
            {
                if((unsigned char)(std::use_facet<std::numpunct<char> >(l).thousands_sep()) > 0x7F)
                {
                    l=add_facet(l,new numpunct_utf8(l.name().c_str()));
                }
                if((unsigned char)(std::use_facet<std::moneypunct<char,true> >(l).thousands_sep()) > 0x7F)
                {
                    l=add_facet(l,new moneypunct_utf8<true>(l.name().c_str()));
                }
                if((unsigned char)(std::use_facet<std::moneypunct<char,false> >(l).thousands_sep()) > 0x7F)
                {
                    l=add_facet(l,new moneypunct_utf8<false>(l.name().c_str()));
                }
                return l;
            }

        } // impl


        struct generator::data {
            typedef std::pair<std::string,std::string> locale_id_type;

            struct keycomp { 
                bool operator()(locale_id_type const &left,locale_id_type const &right) const
                {
                    if(left.first == right.first)
                        return left.second < right.second;
                    return left.first < right.first;
                }
            };
            
            typedef std::map<locale_id_type,std::locale,keycomp> cached_type;
            cached_type cached;

            std::string encoding;

            std::vector<std::string> paths;
            std::set<std::string> domains;
            std::string default_domain;
        };

        generator::generator() :
            d(new generator::data())
        {
        }
        generator::~generator()
        {
        }

        void generator::octet_encoding(std::string const &enc)
        {
            d->encoding=enc;
        }
        std::string generator::octet_encoding() const
        {
            return d->encoding;
        }
        
        void generator::add_messages_domain(std::string const &domain)
        {
            if(d->default_domain.empty())
                d->default_domain=domain;
            d->domains.insert(domain);
        }
        void generator::set_default_messages_domain(std::string const &domain)
        {
            add_messages_domain(domain);
            d->default_domain=domain;
        }
        void generator::clear_domains()
        {
            d->default_domain.clear();
            d->domains.clear();
        }
        void generator::add_messages_path(std::string const &path)
        {
            d->paths.push_back(path);
        }
        void generator::clear_paths()
        {
            d->paths.clear();
        }
        void generator::clear_cache()
        {
            d->cached.empty();
        }

        std::locale generator::generate(std::string const &id) const
        {
            return generate(id,d->encoding);
        }

        std::locale generator::generate(std::string const &id,std::string const &input_encoding) const
        {
            static const booster::regex reg("^([a-zA-Z]+)(([\\-_])([a-zA-Z]+))?(\\.([0-9a-zA-Z_\\-]+))?(@([0-9a-zA-Z_\\-]+))?$");

            booster::cmatch m;
            std::string language="C",country,variant,encoding,sep;

            if(booster::regex_match(id.c_str(),m,reg)) {
                language=m[1];
                sep=m[3];
                country=m[4];
                encoding=m[6];
                variant=m[8];
            }

            if(encoding.empty())
                encoding = input_encoding;
            if(encoding.empty())
                encoding = "UTF-8";
            
            std::vector<std::string> trys;

            if(!variant.empty()) {
                trys.push_back(language+sep+country+"."+encoding+"@" + variant);
                trys.push_back(language+"."+encoding+"@" + variant);
            }
            trys.push_back(language+sep+country+"."+encoding);
            trys.push_back(language+"."+encoding);

            std::locale the_locale=std::locale::classic();

            for(unsigned i=0;i<trys.size();i++) {
                try {
                    the_locale = std::locale(trys[i].c_str());
                    break;
                }
                catch(std::exception const &e) {
                }
            }

            the_locale = add_facet(the_locale,new info(language,country,variant,encoding));
            
            return add_facets(the_locale);

        }

        void generator::preload(std::string const &id) 
        {
            d->cached[data::locale_id_type(id,"")]=generate(id);
        }
        void generator::preload(std::string const &id,std::string const &encoding)
        {
            d->cached[data::locale_id_type(id,encoding)]=generate(id,encoding);
        }
        
        std::locale generator::add_facets(std::locale const &source) const
        {
            std::locale result=source;
            info const &inf=std::use_facet<info>(source);

            result=add_facet(result,new num_format());

            if(!d->default_domain.empty() && !d->paths.empty()) {
                std::vector<std::string> domains;
                domains.push_back(d->default_domain);
                for(std::set<std::string>::const_iterator p=d->domains.begin(),e=d->domains.end();p!=e;++p) {
                    if(*p!=d->default_domain) {
                        domains.push_back(*p);
                    }
                }
                result=add_facet(result,message_format<char>::create(inf,domains,d->paths));
            }

            if(inf.utf8())
                result=impl::workaround_utf8_bug(result);

            return result;
        }
        
        std::locale generator::get(std::string const &id) const
        {
            data::cached_type::const_iterator p=d->cached.find(data::locale_id_type(id,""));            
            if(p==d->cached.end())
                return generate(id);
            else
                return p->second;
        }
        std::locale generator::get(std::string const &id,std::string const &encoding) const
        {
            data::cached_type::const_iterator p=d->cached.find(data::locale_id_type(id,encoding));
            if(p==d->cached.end())
                return generate(id,encoding);
            else
                return p->second;
        }
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
