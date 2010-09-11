//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/config.h>
#include <booster/locale/message.h>
#include <booster/locale/gnu_gettext.h>
#include <booster/shared_ptr.h>
#include <booster/locale/encoding.h>
#if BOOSTER_VERSION >= 103600
#include <booster/unordered_map.h>
#else
#include <map>
#endif


#include "mo_hash.h"
#include "mo_lambda.h"

#include <stdio.h>
#include <iostream>

#include <string.h>

namespace booster {
    namespace locale {
        namespace gnu_gettext {
            
            class c_file {
                c_file(c_file const &);
                void operator=(c_file const &);
            public:
                
                FILE *file;

                c_file() : 
                    file(0)
                {
                }
                ~c_file()
                {
                    close();
                }

                void close()
                {
                    if(file) {
                        fclose(file);
                        file=0;
                    }
                }

                #if defined(BOOSTER_WIN_NATIVE)

                bool open(std::string const &file_name)
                {
                    close();

                    /// Under windows we have to use "_wfopen" to get
                    /// access to path's with Unicode in them
                    ///
                    /// As not all standard C++ libraries support nonstandard std::istream::open(wchar_t const *)
                    /// we would use old and good stdio and _wfopen CRTL functions
                    ///

                    ///
                    /// So in order to distinguish between local encoding and UTF encoding we use BOM
                    ///

                    if(file_name.compare(0,3,"\xEF\xBB\xBF")==0)
                    {
                        std::wstring wfile_name = conv::to_utf<wchar_t>(file_name.substr(3),"UTF-8");
                        file = _wfopen(wfile_name.c_str(),L"rb");
                    }
                    else {
                        file = fopen(file_name.c_str(),"rb");
                    }

                    return file!=0;
                }

                #else // POSIX systems do not have all this Wide API crap, as native codepages are UTF-8
                
                bool open(std::string const &file_name)
                {
                    close();

                    file = fopen(file_name.c_str(),"rb");

                    return file!=0;
                }

                #endif

            };

            class mo_file {
            public:
                typedef std::pair<char const *,char const *> pair_type;
                
                mo_file(FILE *file) :
                    native_byteorder_(true),
                    size_(0)
                {
                    load_file(file);
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
                void load_file_direct(FILE *file)
                {
                    uint32_t magic=0;
                    fread(&magic,4,1,file);
                    
                    if(magic == 0x950412de)
                        native_byteorder_ = true;
                    else if(magic == 0xde120495)
                        native_byteorder_ = false;
                    else
                        throw std::runtime_error("Invalid file format");
                    
                    fseek(file,0,SEEK_END);
                    long len=ftell(file);
                    if(len < 0) {
                        throw std::runtime_error("Wrong file object");
                    }
                    fseek(file,0,SEEK_SET);
                    vdata_.resize(len+1,0); // +1 to make sure the vector is not empty
                    if(fread(&vdata_.front(),1,len,file)!=unsigned(len))
                        throw std::runtime_error("Failed to read file");
                    data_ = &vdata_[0];
                    file_size_ = len;
                }
                
                void load_file(FILE *file)
                {
                   load_file_direct(file);
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
            class converter {
            public:
                converter(std::string /*out_enc*/,std::string in_enc) :
                    in_(in_enc)
                {
                }

                std::basic_string<CharType> operator()(char const *begin,char const *end)
                {
                    return conv::to_utf<CharType>(begin,end,in_,conv::stop);
                }

            private:
                std::string in_;
            };
            
            template<>
            class converter<char> {
            public:
                converter(std::string out_enc,std::string in_enc) :
                    out_(out_enc),
                    in_(in_enc)
                {
                }

                std::string operator()(char const *begin,char const *end)
                {
                    return conv::between(begin,end,out_,in_,conv::stop);
                }

            private:
                std::string out_,in_;
            };

            template<typename CharType>
            class mo_message : public message_format<CharType> {

                typedef std::basic_string<CharType> string_type;
                #if BOOSTER_VERSION >= 103600
                typedef booster::unordered_map<std::string,string_type> catalog_type;
                #else
                typedef std::map<std::string,string_type> catalog_type;
                #endif
                typedef std::vector<catalog_type> catalogs_set_type;
                typedef std::map<std::string,int> domains_map_type;
            public:

                typedef std::pair<CharType const *,CharType const *> pair_type;

                virtual CharType const *get(int domain_id,char const *context,char const *id) const
                {
                    return get_string(domain_id,context,id).first;
                }

                virtual CharType const *get(int domain_id,char const *context,char const *single_id,int n) const
                {
                    pair_type ptr = get_string(domain_id,context,single_id);
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

                mo_message(messages_info const &inf)
                {
                    std::string language = inf.language;
                    std::string variant = inf.variant;
                    std::string country = inf.country;
                    std::string encoding = inf.encoding;
                    std::string lc_cat = inf.locale_category;
                    std::vector<std::string> const &domains = inf.domains;
                    std::vector<std::string> const &search_paths = inf.paths;
                    
                    //
                    // List of fallbacks: en_US@euro, en@euro, en_US, en. 
                    //
                    std::vector<std::string> paths;


                    if(!variant.empty() && !country.empty()) 
                        paths.push_back(language + "_" + country + "@" + variant);

                    if(!variant.empty()) 
                        paths.push_back(language + "@" + variant);

                    if(!country.empty())
                        paths.push_back(language + "_" + country);

                    paths.push_back(language);

                    catalogs_.resize(domains.size());
                    mo_catalogs_.resize(domains.size());
                    plural_forms_.resize(domains.size());


                    for(unsigned id=0;id<domains.size();id++) {
                        std::string domain=domains[id];
                        domains_[domain]=id;


                        bool found=false; 
                        for(unsigned j=0;!found && j<paths.size();j++) {
                            for(unsigned i=0;!found && i<search_paths.size();i++) {
                                std::string full_path = search_paths[i]+"/"+paths[j]+"/" + lc_cat + "/"+domain+".mo";

                                found = load_file(full_path,encoding,id);
                            }
                        }
                    }
                }

                virtual ~mo_message()
                {
                }

            private:
                int compare_encodings(std::string const &left,std::string const &right)
                {
                    return convert_encoding_name(left).compare(convert_encoding_name(right)); 
                }

                std::string convert_encoding_name(std::string const &in)
                {
                    std::string result;
                    for(unsigned i=0;i<in.size();i++) {
                        char c=in[i];
                        if('A' <= c && c<='Z')
                            c=c-'A' + 'a';
                        else if(('a' <= c && c<='z') || ('0' <= c && c<='9'))
                            ;
                        else
                            continue;
                        result+=c;
                    }
                    return result;
                }


                bool load_file(std::string file_name,std::string encoding,int id)
                {
                    c_file the_file;

                    the_file.open(file_name);

                    if(!the_file.file)
                        return false;

                    std::auto_ptr<mo_file> mo(new mo_file(the_file.file));

                    the_file.close();
                    
                    std::string plural = extract(mo->value(0).first,"plural=","\r\n;");

                    std::string mo_encoding = extract(mo->value(0).first,"charset="," \r\n;");

                    if(mo_encoding.empty())
                        throw std::runtime_error("Invalid mo-format, encoding is not specified");

                    if(!plural.empty()) {
                        std::auto_ptr<lambda::plural> ptr=lambda::compile(plural.c_str());
                        plural_forms_[id] = ptr;
                    }

                    if( sizeof(CharType) == 1
                        && compare_encodings(mo_encoding.c_str(),encoding.c_str()) == 0
                        && mo->has_hash())
                    {
                        mo_catalogs_[id]=mo;
                    }
                    else {
                        converter<CharType> cvt(encoding,mo_encoding);
                        for(unsigned i=0;i<mo->size();i++) {
                            mo_file::pair_type tmp = mo->value(i);
                            catalogs_[id][mo->key(i)]=cvt(tmp.first,tmp.second);
                        }
                    }
                    return true;

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




                pair_type get_string(int domain_id,char const *context,char const *in_id) const
                {
                    char const *id = in_id;
                    std::string cid;

                    if(context!=0 && *context!=0) {
                        cid.reserve(strlen(context) + strlen(in_id) + 1);
                        cid.append(context);
                        cid+='\4'; // EOT
                        cid.append(in_id);
                        id = cid.c_str();
                    }

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
                std::vector<booster::shared_ptr<mo_file> > mo_catalogs_;
                std::vector<booster::shared_ptr<lambda::plural> > plural_forms_;
                domains_map_type domains_;

                
            };

            template<>
            message_format<char> *create_messages_facet(messages_info &info)
            {
                return new mo_message<char>(info);
            }

            template<>
            message_format<wchar_t> *create_messages_facet(messages_info &info)
            {
                return new mo_message<wchar_t>(info);
            }
            
            #ifdef BOOSTER_HAS_CHAR16_T

            template<>
            message_format<char16_t> *create_messages_facet(messages_info &info)
            {
                return new mo_message<char16_t>(info);
            }
            #endif
            
            #ifdef BOOSTER_HAS_CHAR32_T

            template<>
            message_format<char32_t> *create_messages_facet(messages_info &info)
            {
                return new mo_message<char32_t>(info);
            }
            #endif


        } /// gnu_gettext

    } // locale
} // boost
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

