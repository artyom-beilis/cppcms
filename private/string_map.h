///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_STRING_MAP_H
#define CPPCMS_IMPL_STRING_MAP_H

#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <limits>
#include <booster/noncopyable.h>

namespace cppcms {
	namespace impl {
		class string_pool  : public booster::noncopyable {
		public:
			string_pool(size_t page_size = 2048) : 
				page_size_(page_size),
				pages_(0),
				free_space_(0),
				data_(0)
			{
				add_page();
			}
			void clear()
			{
				destroy();
				add_page();
			}
			~string_pool()
			{
				destroy();
			}
			char *alloc(size_t n)
			{
				char *s = allocate_space(n);
				memset(s,0,n);
				return s;
			}
			char *add(std::string const &s)
			{
				return add(s.c_str());
			}
			char *add_substr(std::string const &s,size_t pos = 0,size_t len=std::string::npos)
			{
				if(pos > s.size())
					return add("");
				return add(s.c_str() + pos,len);
			}
			char *add(char const *s)
			{
				return add_bounded_string(s,strlen(s));
			}
			char *add(char const *b,char const *e)
			{
				return add(b,e-b);
			}
			char *add(char const *s,size_t len)
			{
				char const *tmp = s;
				size_t real_len = 0;
				while(real_len < len && *tmp!=0) {
					real_len ++;
					tmp++;
				}
				return add_bounded_string(s,real_len);
			}

		private:
			void destroy()
			{
				while(pages_) {
					page *p = pages_;
					pages_ = pages_->next;
					free(p);
				}
			}
			/// data
			struct page {
				page *next;
				char data[1];
			};

			size_t page_size_;
			page *pages_;
			size_t free_space_;
			char *data_;

			char *allocate_space(size_t size)
			{
				if(size * 2 > page_size_) {
					page *p=(page *)malloc(size + sizeof(page));
					if(!p)
						throw std::bad_alloc();
					p->next = pages_->next;
					pages_->next = p;
					return p->data;
				}
				if(size > free_space_) {
					add_page();
				}
				char *result = data_;
				data_+=size;
				free_space_ -= size;
				return result;
			}
			char *add_bounded_string(char const *str,size_t size)
			{
				char *s=allocate_space(size + 1);
				memcpy(s,str,size);
				s[size]=0;
				return s;
			}
			void add_page()
			{
				page *p=(page *)malloc(sizeof(page) + page_size_);
				if(!p)
					throw std::bad_alloc();
				p->next = pages_;
				pages_ = p;
				data_ = p->data;
				free_space_ = page_size_;
			}


		};

		class string_map {
		public:
			struct entry {
				char const *key;
				char const *value;
				entry(char const *k ="",char const *v="") : key(k),value(v) {}
				bool operator<(entry const &other) const
				{
					return strcmp(key,other.key) < 0;
				}
				bool operator==(entry const &other) const
				{
					return strcmp(key,other.key) == 0;
				}
			};

			string_map() : sorted_(true) 
			{
				data_.reserve(64);
			}
			typedef std::vector<entry>::iterator iterator;
			iterator begin()
			{
				sort();
				return data_.begin();
			}
			iterator end()
			{
				sort();
				return data_.end();
			}
			void add(char const *key,char const *value)
			{
				data_.push_back(entry(key,value));
				sorted_=false;
			}
			void sort()
			{
				if(!sorted_) {
					std::sort(data_.begin(),data_.end());
					sorted_ = true;
				}
			}
			char const *get(char const *ckey)
			{
				sort();
				entry key(ckey);
				iterator p = std::lower_bound(data_.begin(),data_.end(),key);
				if(p!=data_.end() && *p==key)
					return p->value;
				return 0;
			}
			char const *get_safe(char const *key)
			{
				char const *value =get(key);
				if(value)
					return value;
				return "";
			}
			void clear()
			{
				data_.clear();
				sorted_ = false;
			}
		private:
			bool sorted_;
			std::vector<entry> data_;
		};

	} // impl
}


#endif
