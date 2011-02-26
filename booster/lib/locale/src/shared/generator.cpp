//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/locale/generator.h>
#include <booster/locale/encoding.h>
#include <booster/locale/localization_backend.h>
#include <map>
#include <vector>
#include <algorithm>
#include <booster/shared_ptr.h>
#include <booster/thread.h>

namespace booster {
    namespace locale {
        struct generator::data {
            data(localization_backend_manager const &mgr)  :
                cats(all_categories),
                chars(all_characters),
                caching_enabled(false),
                backend_manager(mgr)
            {
            }

            typedef std::map<std::string,std::locale> cached_type;
            mutable cached_type cached;
            mutable booster::mutex cached_lock;

            unsigned cats;
            unsigned chars;

            bool caching_enabled;

            std::vector<std::string> paths;
            std::vector<std::string> domains;

            std::map<std::string,std::vector<std::string> > options;

            localization_backend_manager backend_manager;

        };

        generator::generator(localization_backend_manager const &mgr) :
            d(new generator::data(mgr))
        {
        }
        generator::generator() :
            d(new generator::data(localization_backend_manager::global()))
        {
        }
        generator::~generator()
        {
        }

        unsigned generator::categories() const
        {
            return d->cats;
        }
        void generator::categories(unsigned t)
        {
            d->cats=t;
        }

        void generator::characters(unsigned t)
        {
            d->chars=t;
        }
        
        unsigned generator::characters() const
        {
            return d->chars;
        }

        void generator::add_messages_domain(std::string const &domain)
        {
            if(std::find(d->domains.begin(),d->domains.end(),domain) == d->domains.end())
                d->domains.push_back(domain);
        }
        
        void generator::set_default_messages_domain(std::string const &domain)
        {
            std::vector<std::string>::iterator p;
            if((p=std::find(d->domains.begin(),d->domains.end(),domain)) == d->domains.end()) {
                d->domains.erase(p);
            }
            d->domains.insert(d->domains.begin(),domain);
        }

        void generator::clear_domains()
        {
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
            d->cached.clear();
        }

        std::locale generator::generate(std::string const &id) const
        {
            std::locale base=std::locale::classic();

            return generate(base,id);
        }

        std::locale generator::generate(std::locale const &base,std::string const &id) const
        {
            if(d->caching_enabled) {
                booster::unique_lock<booster::mutex> guard(d->cached_lock);
                data::cached_type::const_iterator p = d->cached.find(id);
                if(p!=d->cached.end()) {
                    return p->second;
                }
            }
            shared_ptr<localization_backend> backend(d->backend_manager.get());
            set_all_options(backend,id);

            std::locale result = base;
            unsigned facets = d->cats;
            unsigned chars = d->chars;

            for(unsigned facet = per_character_facet_first; facet <= per_character_facet_last && facet!=0; facet <<=1) {
                if(!(facets & facet))
                    continue;
                for(unsigned ch = character_first_facet ; ch<=character_last_facet;ch <<=1) {
                    if(!(ch & chars))
                        continue;
                    result = backend->install(result,facet,ch);
                }
            }
            for(unsigned facet = non_character_facet_first; facet <= non_character_facet_last && facet!=0; facet <<=1) {
                if(!(facets & facet))
                    continue;
                result = backend->install(result,facet);
            }
            if(d->caching_enabled) {
                booster::unique_lock<booster::mutex> guard(d->cached_lock);
                data::cached_type::const_iterator p = d->cached.find(id);
                if(p==d->cached.end()) {
                    d->cached[id] = result;
                }
            }
            return result;
        }

        bool generator::locale_cache_enabled() const
        {
            return d->caching_enabled;
        }
        void generator::locale_cache_enabled(bool enabled) 
        {
            d->caching_enabled = enabled;
        }
        
        void generator::set_all_options(shared_ptr<localization_backend> backend,std::string const &id) const
        {
            backend->set_option("locale",id);
            for(unsigned i=0;i<d->domains.size();i++)
                backend->set_option("message_application",d->domains[i]);
            for(unsigned i=0;i<d->paths.size();i++)
                backend->set_option("message_path",d->paths[i]);
        }
        
    } // locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4 
