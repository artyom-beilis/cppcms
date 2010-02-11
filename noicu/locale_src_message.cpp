//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define CPPCMS_LOCALE_SOURCE
#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/config.hpp>
#else // Internal Boost
#   include <cppcms_boost/config.hpp>
    namespace boost = cppcms_boost;
#endif
#include "locale_info.h"
#include "locale_message.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/shared_ptr.hpp>
#else // Internal Boost
#   include <cppcms_boost/shared_ptr.hpp>
#endif
#if 1
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/unordered_map.hpp>
#else // Internal Boost
#   include <cppcms_boost/unordered_map.hpp>
#endif
#else
#include <map>
#endif


#include "locale_src_uconv.hpp"
#include "locale_src_mo_hash.hpp"
#include "locale_src_mo_lambda.hpp"

#include <fstream>
#include <iostream>

#include <string.h>

namespace cppcms {
    namespace locale {


        namespace impl {
            class mo_file {
            public:
                typedef std::pair<char const *,char const *> pair_type;
                
                mo_file(std::string file_name) :
                    native_byteorder_(true),
                    size_(0)
                {
                    load_file(file_name);
                    // Read all format sizes
                    size_=get(8);
                    keys_offset_=get(12);
                    translations_offset_=get(16);
                    hash_size_=get(20);
                    hash_offset_=get(24);
                }

                pair_type find(char const *key_in) const
                {
                    pair_type null_pair((char const *)0,(char const *)0);
                    if(hash_size_==0)
                        return null_pair;
                    uint32_t hkey = pj_winberger_hash_function(key_in);
                    uint32_t incr = 1 + hkey % (hash_size_-2);
                    hkey %= hash_size_;
                    uint32_t orig=hkey;
                    
                    
                    do {
                        uint32_t idx = get(hash_offset_ + 4*hkey);
                        /// Not found
                        if(idx == 0)
                            return pair_type((char const *)0,(char const *)0);
                        /// If equal values return translation
                        if(strcmp(key(idx-1),key_in)==0)
                            return value(idx-1);
                        /// Rehash
                        hkey=(hkey + incr) % hash_size_;
                    } while(hkey!=orig);
                    return null_pair;
                }
                
                char const *key(int id) const
                {
                    uint32_t off = get(keys_offset_ + id*8 + 4);
                    return data_ + off;
                }

                pair_type value(int id) const
                {
                    uint32_t len = get(translations_offset_ + id*8);
                    uint32_t off = get(translations_offset_ + id*8 + 4);
                    return pair_type(&data_[off],&data_[off]+len);
                }

                bool has_hash() const
                {
                    return hash_size_ != 0;
                }

                size_t size() const
                {
                    return size_;
                }

                bool empty()
                {
                    return size_ == 0;
                }

            private:
                void load_file_direct(std::string file_name)
                {
                    std::ifstream file(file_name.c_str(),std::ios::binary);
                    if(!file)
                        throw std::runtime_error("No such file");
                    //
                    // Check file format
                    //
                    
                    uint32_t magic=0;
                    file.read(reinterpret_cast<char *>(&magic),sizeof(magic));
                    
                    if(magic == 0x950412de)
                        native_byteorder_ = true;
                    else if(magic == 0xde120495)
                        native_byteorder_ = false;
                    else
                        throw std::runtime_error("Invalid file format");
                    

                    // load image of file to memory
                    file.seekg(0, std::ios::end);
                    int len=file.tellg();
                    file.seekg(0, std::ios::beg);
                    vdata_.resize(len,0);
                    file.read(&vdata_.front(),len);
                    if(file.gcount()!=len)
                        throw std::runtime_error("Failed to read file");
                    data_ = &vdata_[0];
                    file_size_ = vdata_.size();
                }
                
                void load_file(std::string file_name)
                {
                   load_file_direct(file_name);
                }

                uint32_t get(unsigned offset) const
                {
                    uint32_t tmp;
                    if(offset > file_size_ - 4) {
                        throw std::runtime_error("Bad file format");
                    }
                    memcpy(&tmp,data_ + offset,4);
                    convert(tmp);
                    return tmp;
                }

                void convert(uint32_t &v) const
                {
                    if(native_byteorder_)
                        return;
                    v =   ((v & 0xFF) << 24)
                        | ((v & 0xFF00) << 8)
                        | ((v & 0xFF0000) >> 8)
                        | ((v & 0xFF000000) >> 24);
                }

                uint32_t keys_offset_;
                uint32_t translations_offset_;
                uint32_t hash_size_;
                uint32_t hash_offset_;

                char const *data_;
                size_t file_size_;
                std::vector<char> vdata_;
                bool native_byteorder_;
                size_t size_;
            };

            template<typename CharType>
            class mo_message : public message_format<CharType> {

                typedef std::basic_string<CharType> string_type;
                #if 1
                typedef boost::unordered_map<std::string,string_type> catalog_type;
                #else
                typedef std::map<std::string,string_type> catalog_type;
                #endif
                typedef std::vector<catalog_type> catalogs_set_type;
                typedef std::map<std::string,int> domains_map_type;
            public:

                typedef std::pair<CharType const *,CharType const *> pair_type;

                virtual CharType const *get(int domain_id,char const *id) const
                {
                    return get_string(domain_id,id).first;
                }

                virtual CharType const *get(int domain_id,char const *single_id,int n) const
                {
                    pair_type ptr = get_string(domain_id,single_id);
                    if(!ptr.first)
                        return 0;
                    int form=0;
                    if(plural_forms_.at(domain_id)) 
                        form = (*plural_forms_[domain_id])(n);
                    else
                        form = n == 1 ? 0 : 1; // Fallback to english plural form

                    CharType const *p=ptr.first;
                    for(int i=0;p < ptr.second && i<form;i++) {
                        p=std::find(p,ptr.second,0);
                        if(p==ptr.second)
                            return 0;
                        ++p;
                    }
                    if(p>=ptr.second)
                        return 0;
                    return p;
                }

                virtual int domain(std::string const &domain) const
                {
                    domains_map_type::const_iterator p=domains_.find(domain);
                    if(p==domains_.end())
                        return -1;
                    return p->second;
                }

                mo_message(info const &inf,std::vector<std::string> const &domains,std::vector<std::string> const &search_paths)
                {
                    std::string encoding = inf.encoding();

                    catalogs_.resize(domains.size());
                    mo_catalogs_.resize(domains.size());
                    plural_forms_.resize(domains.size());
                    
                    for(unsigned id=0;id<domains.size();id++) {
                        std::string domain=domains[id];
                        domains_[domain]=id;
                        //
                        // List of fallbacks: en_US@euro, en@euro, en_US, en. 
                        //
                        static const unsigned paths_no = 4;

                        std::string paths[paths_no] = {
                            std::string(inf.language()) + "_" + inf.country() + "@" + inf.variant(),
                            std::string(inf.language()) + "@" + inf.variant(),
                            std::string(inf.language()) + "_" + inf.country(),
                            std::string(inf.language()),
                        };
                       
                        bool found=false; 
                        for(unsigned j=0;!found && j<paths_no;j++) {
                            for(unsigned i=0;!found && i<search_paths.size();i++) {
                                std::string full_path = search_paths[i]+"/"+paths[j]+"/LC_MESSAGES/"+domain+".mo";
                                
                                found = load_file(full_path,encoding,id);
                            }
                        }
                    }
                }

                virtual ~mo_message()
                {
                }

            private:

                bool load_file(std::string file_name,std::string encoding,int id)
                {
                    try {
                        std::auto_ptr<mo_file> mo(new mo_file(file_name));

                        std::string plural = extract(mo->value(0).first,"plural=","\r\n;");
                        std::string mo_encoding = extract(mo->value(0).first,"charset="," \r\n;");
                        if(mo_encoding.empty())
                            throw std::runtime_error("Invalid mo-format, encoding is not specified");
                        if(!plural.empty()) {
                            std::auto_ptr<lambda::plural> ptr=lambda::compile(plural.c_str());
                            plural_forms_[id] = ptr;
                        }
                        if( sizeof(CharType) == 1
                            && ucnv_compareNames(mo_encoding.c_str(),encoding.c_str()) == 0
                            && mo->has_hash())
                        {
                            mo_catalogs_[id]=mo;
                        }
                        else {
                            converter cvt(encoding,mo_encoding);
                            for(unsigned i=0;i<mo->size();i++) {
                                mo_file::pair_type tmp = mo->value(i);
                                catalogs_[id][mo->key(i)]=cvt(tmp.first,tmp.second);
                            }
                        }
                        return true;

                    }
                    catch(std::exception const &err)
                    {
                        plural_forms_[id].reset();
                        catalogs_[id].clear();
                        return false;
                    }

                }

                static std::string extract(std::string const &meta,std::string const &key,char const *separator)
                {
                    size_t pos=meta.find(key);
                    if(pos == std::string::npos)
                        return "";
                    pos+=key.size(); /// size of charset=
                    size_t end_pos = meta.find_first_of(separator,pos);
                    return meta.substr(pos,end_pos - pos);
                }


                class converter {
                public:
                    converter(std::string out_enc,std::string in_enc) :
                        out_(out_enc),
                        in_(in_enc)
                    {
                    }

                    std::basic_string<CharType> operator()(char const *begin,char const *end)
                    {
                        return out_.std(in_.icu(begin,end));
                    }

                private:
                    icu_std_converter<CharType> out_;
                    icu_std_converter<char> in_;
                };


                pair_type get_string(int domain_id,char const *id) const
                {
                    pair_type null_pair((CharType const *)0,(CharType const *)0);
                    if(domain_id < 0 || size_t(domain_id) >= catalogs_.size())
                        return null_pair;
                    if(mo_catalogs_[domain_id]) {
                        mo_file::pair_type p=mo_catalogs_[domain_id]->find(id);
                        return pair_type(reinterpret_cast<CharType const *>(p.first),
                                         reinterpret_cast<CharType const *>(p.second));
                    }
                    else {
                        catalog_type const &cat = catalogs_[domain_id];
                        typename catalog_type::const_iterator p = cat.find(id);
                        if(p==cat.end())
                            return null_pair;
                        return pair_type(p->second.data(),p->second.data()+p->second.size());
                    }
                }

                catalogs_set_type catalogs_;
                std::vector<boost::shared_ptr<mo_file> > mo_catalogs_;
                std::vector<boost::shared_ptr<lambda::plural> > plural_forms_;
                domains_map_type domains_;

                
            };
        } /// impl

        
        //
        // facet IDs and specializations
        //

        std::locale::id base_message_format<char>::id;

        template<>
        message_format<char> *message_format<char>::create( info const &inf,
                                                            std::vector<std::string> const &domains,
                                                            std::vector<std::string> const &paths)
        {
            return new impl::mo_message<char>(inf,domains,paths);
        }

        #ifndef CPPCMS_NO_STD_WSTRING
        std::locale::id base_message_format<wchar_t>::id;
        
        template<>
        message_format<wchar_t> *message_format<wchar_t>::create(   info const &inf,
                                                                    std::vector<std::string> const &domains,
                                                                    std::vector<std::string> const &paths)
        {
            return new impl::mo_message<wchar_t>(inf,domains,paths);
        }
        #endif
        
        #ifdef CPPCMS_HAS_CHAR16_T
        std::locale::id base_message_format<char16_t>::id;

        template<>
        message_format<char16_t> *message_format<char16_t>::create( info const &inf,
                                                                    std::vector<std::string> const &domains,
                                                                    std::vector<std::string> const &paths)
        {
            return new impl::mo_message<char16_t>(inf,domains,paths);
        }
        #endif
        
        #ifdef CPPCMS_HAS_CHAR32_T
        std::locale::id base_message_format<char32_t>::id;

        template<>
        message_format<char32_t> *message_format<char32_t>::create( info const &inf,
                                                                    std::vector<std::string> const &domains,
                                                                    std::vector<std::string> const &paths)
        {
            return new impl::mo_message<char32_t>(inf,domains,paths);
        }
        #endif

    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

