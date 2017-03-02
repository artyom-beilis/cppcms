///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_PRIVATE_BUDDY_ALLOCATOR_H
#define CPPCMS_PRIVATE_BUDDY_ALLOCATOR_H

#include <cppcms/defs.h>
#include <stddef.h>
#include <assert.h>
#include <memory.h>

#ifdef DEBUG_ALLOCATOR
#include <stdio.h>
#define LOG(...) do { printf("%5d:",__LINE__); printf( __VA_ARGS__ ); printf("\n"); } while(0)
#else
#define LOG(...) do {} while(0)
#endif

#ifdef TEST_ALLOCATOR
#include <map>
#include <utility>
#endif
namespace cppcms {
namespace impl {
class buddy_allocator {
	struct page;
public:
	static const int alignment_bits = (sizeof(void*) > 4 ? 4 : 3);
	static const size_t alignment = (1 << alignment_bits); // 8 or 16 at 32 and 64 bit platform

	buddy_allocator(size_t memory_size)
	{
		assert(sizeof(*this) <= memory_size);
		assert(sizeof(page) <= alignment * 2);
		assert(sizeof(*this) % alignment == 0);
		memory_size_ = memory_size - sizeof(*this);
		max_bit_size_ = -1;
		memset(free_list_,0,sizeof(free_list_));
		LOG("Usable memory %zd",memory_size_);
		char *pos = memory();
		size_t reminder = memory_size_;
		for(;;) {
			int bits = containts_bits(reminder);
			if(bits < alignment_bits + 1)
				break;
			size_t page_size = size_t(1) << bits;
			page *p=reinterpret_cast<page *>(pos);
			reminder -= page_size;
			LOG("Added chunk of size %zd (%d) at pos %zx reminder %zd",page_size,bits,pos - memory(),reminder);
			pos+= page_size;
			p->bits = bits;
			p->prev = 0;
			p->next = 0;
			free_list_[bits] = p;
			if(max_bit_size_ == -1)
				max_bit_size_ = bits;
		}
	}


	void *malloc(size_t required_size)
	{
		size_t n = ((required_size+alignment-1)/alignment + 1)*alignment;
		int bits = get_bits(n);

		
		LOG("Allocating chunk of size %zd (%d) for request of %zd",n,bits,required_size);
		page *p = page_alloc(bits);
		LOG("Got %zx",(p?(char*)(p) - memory():0));
		if(!p)
			return 0;
		void *r = reinterpret_cast<char *>(p) + alignment;
		return r;
	}

	void free(void *ptr)
	{
		if(!ptr)
			return;
		page *p = reinterpret_cast<page *>(static_cast<char *>(ptr) - alignment);
		assert(p->bits & page_in_use);
		LOG("Freeing page %zx with bits %d",(char *)(p) - memory() , p->bits - page_in_use);
		free_page(p);
	}

	size_t total_free_memory()
	{
		size_t total = 0;
		for(unsigned i=0;i<sizeof(void*)*8;i++) {
			total += total_free_at(i);
		}
		return total;
	}
	size_t max_free_chunk()
	{
		for(int bits=sizeof(void*)*8-1;bits>0;bits--) {
			if(free_list_[bits])
				return total_free_at(bits);
		}
		return 0;
	}
	size_t total_free_at(int bits)
	{
		size_t count=0;
		for(page *p=free_list_[bits];p;p=p->next)
			count++;
		return count * ((size_t(1) << bits) - alignment);
	}

#ifdef TEST_ALLOCATOR
	void test_free() {
		char *p=memory();
		for(int i=max_bit_size_;i>=0;i--) {
			if(free_list_[i]) {
				TEST((char*)free_list_[i]==p);
				TEST(free_list_[i]->bits == i);
				p += 1ul << free_list_[i]->bits;
			}
		}
		TEST(p<= memory() + memory_size_);
		TEST(memory() + memory_size_ - p < int(alignment * 2));
	}
	void test_and_get_free_pages(std::map<page *,bool> *all_pages=0)
	{
		for(int i=0;i<int(sizeof(void*)*8);i++) {
			if(i+1 < alignment_bits)
				TEST(free_list_[i]==0);
			if(i>max_bit_size_)
				TEST(free_list_[i]==0);

			for(page *p=free_list_[i];p;p=p->next) {
				// check bit marks
				TEST(p->bits == i);
				// check linked list
				if(p==free_list_[i])
					TEST(p->prev == 0);
				else
					TEST(p->prev->next == p);
				if(p->next != 0)
					TEST(p->next->prev == p);
				if(all_pages) {
					TEST(all_pages->find(p) == all_pages->end());
					all_pages->insert(std::make_pair(p,true));
				}
				page *buddy = get_buddy(p);
				if(buddy) {
					TEST((buddy->bits & page_in_use) || (buddy->bits < p->bits));
					TEST((buddy->bits & 0xFF) <= p->bits);
					if(all_pages && (buddy->bits & page_in_use))
						TEST(all_pages->find(buddy) == all_pages->end());
				}
			}
		}
	}
	void test_consistent(void * const *allocated,size_t allocated_size)
	{
		std::map<page *,bool> all_pages;
		test_and_get_free_pages(&all_pages);
		for(size_t i=0;i<allocated_size;i++) {
			if(allocated[i]==0)
				continue;
			page *p=reinterpret_cast<page *>(static_cast<char *>(allocated[i]) - alignment);
			TEST(p->bits & page_in_use);
			TEST(all_pages.find(p)==all_pages.end());
			all_pages[p]=false;
		}
		size_t pos = 0;
		for(std::map<page *,bool>::const_iterator pg=all_pages.begin();pg!=all_pages.end();++pg) {
			page *p=pg->first;
			bool is_free = pg->second;
			TEST(is_free == !(p->bits & page_in_use));
			size_t page_pos = reinterpret_cast<char *>(pg->first) - memory();
			TEST(pos == page_pos);
			size_t page_size = size_t(1) << (pg->first->bits & 0xFF);
			pos+= page_size;
			TEST(pos <= memory_size_);
		}
		TEST(memory_size_ - pos < size_t(alignment * 2));
	}
	void test_consistent()
	{
		test_and_get_free_pages(0);
	}
#endif

private:

	static const int page_in_use = 0x100;

	struct page {
		int bits;
		page  *next;
		page  *prev;
	};

	static int containts_bits(size_t n)
	{
		for(int i=sizeof(n)*8-2;i>0;i--)  {
			size_t upper = size_t(1) << (i + 1);
			size_t lower = upper / 2;
			if(lower <= n && n< upper)
				return i;
		}
		return -1;
		
	}
	static int get_bits(size_t n)
	{
		int i;
		for(i=0;i<int(sizeof(n) * 8);i++) {
			if( (1ull << i) >= n)
				break;
		}
		return i;
	}


	page *page_alloc(int bit_size)
	{
		LOG("Allocating page %d bits",bit_size);
		if(bit_size > max_bit_size_) {
			LOG("Too big size requested %d > %d",bit_size,max_bit_size_);
			return 0;
		}

		page *result = 0;
		
		if(free_list_[bit_size]==0) {
			LOG("No page for bits %d, splitting",bit_size);
			page *to_split = page_alloc(bit_size + 1);
			if(!to_split)
				return 0;

			page *unused = reinterpret_cast<page *>(reinterpret_cast<char *>(to_split) + (size_t(1)<<bit_size));

			unused->prev = 0;
			unused->next = 0;
			unused->bits = bit_size;
			

			free_list_[bit_size] = unused;

			result = to_split;
			LOG("Got %zx; %zx is spare",(char*)to_split - memory(),(char*)unused - memory());
		}
		else {
			result = free_list_[bit_size];
			free_list_[bit_size]=result->next;
			if(free_list_[bit_size]) 
				free_list_[bit_size]->prev = 0;
			LOG("Using free page %zx, disconnecting",(char*)result - memory());
		}

		result->next = 0;
		result->prev = 0;
		result->bits = bit_size + page_in_use;
		LOG("Result is %zx",(char*)result - memory());
		return result;
	}
	page *get_buddy(page *p)
	{
		size_t p_ptr = reinterpret_cast<char *>(p) - memory();
		size_t p_len = size_t(1) << p->bits;
		size_t b_ptr = p_len ^ p_ptr;
		if(b_ptr + p_len > memory_size_)
			return 0;
		return reinterpret_cast<page *>(b_ptr + memory());
	}
	void free_page(page *p)
	{
		assert(p->bits & page_in_use);
		for(;;) {
			p->bits -= page_in_use;
			int bits = p->bits;
			LOG("Freing page with bits %d at %zx",bits,(char*)(p) - memory());
			page *buddy = get_buddy(p);
			if(buddy != 0 && buddy->bits == bits) {
				page *bnext = buddy->next;
				page *bprev = buddy->prev;
				if(bnext)
					bnext->prev = bprev;
				if(bprev)
					bprev->next = bnext;
				if(bprev==0) {
					free_list_[bits]=bnext;
				}
				if(buddy < p)
					p=buddy;
				LOG("Found free buddy merging to %zx with bits %d freeing it as well",(char*)(p)-memory(),bits+1);
				p->bits = (bits+1) + page_in_use;
				continue; // tail recursion
			}
			else {
				LOG("No buddy avalible - adding to free list");
				p->next = free_list_[bits];
				p->prev = 0;
				if(p->next)
					p->next->prev = p;
				free_list_[bits] = p;
				return;
			}
		}
	}
	char *memory()
	{
		return reinterpret_cast<char *>(this) + sizeof(*this);
	}
private:
	// DATA

	page *free_list_[sizeof(void*)*8];	// 16 always
	size_t memory_size_;	
	int max_bit_size_; // at least sizeof(size_t)	
	CPPCMS_UNUSED_MEMBER size_t padding_for_alignment_[2]; 
};
} // impl
} // cppcms

#endif
