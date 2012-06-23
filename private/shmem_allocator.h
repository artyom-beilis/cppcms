///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_SHMEM_ALLOCATOR_H
#define CPPCMS_IMPL_SHMEM_ALLOCATOR_H
#include <assert.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <cppcms/cstdint.h>
#include <cppcms/config.h>

#include "basic_allocator.h"
#include "posix_util.h"
#include <cppcms/cppcms_error.h>

#include <string>
#include <limits>


namespace cppcms {
namespace impl {
namespace details {

class  memory_manager {
public:
	
	memory_manager(size_t n,int bits=12)
	{
		init(n,bits);
	}

	void *malloc(size_t n)
	{
		return slab_alloc(n);
	}

	void free(void *p)
	{
		slab_free(p);
	}

	size_t get_free_memory()
	{
		size_t max_block = 0;
		size_t last_free = 0;
		for(size_t i=0;i<map_size;i++) {
			if(map[i]==0) {
				last_free ++;
				if(last_free > max_block)
					max_block = last_free;
			}
			else {
				last_free=0;
			}
		}
		return max_block * super_page_size;
	}

	#ifdef TEST_ALLOCATOR
	void test_free()
	{
		ptest_free();
	}
	#endif
private:

	class segment;

	char *memory_start;
	size_t map_size;
	int page_bits;
	size_t page_size;
	size_t super_page_size;
	int slab_bits;
	
	static const int align_bits = 4;
	static const size_t align = 1 << align_bits;
	static const int min_bits = align_bits + 1;

	
	segment *all[sizeof(void *)*8][2];
	
	unsigned long long map[1];

	static size_t align_addr(size_t pos)
	{
		return ((pos + (align-1)) >> align_bits) << align_bits;
	}

	static void *align_addr(void *p)
	{
		size_t pos = reinterpret_cast<size_t>(p);
		pos = align_addr(pos);
		return reinterpret_cast<void*>(pos);
	}
	
	void init(size_t n,int pb=12)
	{
		page_bits = pb;
		page_size = 1 << page_bits; // 4096
		super_page_size = page_size * 64;
		slab_bits = page_bits - min_bits;

		
		map_size = (n - align_addr(sizeof(*this))) / (super_page_size + 8);

		// sizeof this contains 1 more value for map 
		memory_start = (char *)(this) + align_addr(sizeof(*this) + map_size * 8);

		if(n < align_addr(sizeof(*this)) || map_size == 0) {
			throw cppcms::cppcms_error("The provided memory region is too small, increase cache.memory");
		}
		
		assert((char *)&map[map_size] <= memory_start);
		assert(align_addr(memory_start) == memory_start);
		assert(memory_start + super_page_size * map_size <= (char*)(this) + n);
	
		memset(map,0,map_size * 8);
	}
	
	
	class segment {
	public:
		segment(int bits,int pos) 
		{
			memset(this,0,sizeof(*this));
			page_size_in_bits_ = bits;
			segment_offset_ = pos;
		}
		int bits() const
		{
			return page_size_in_bits_;
		}
		int segment_size() const
		{
			return 1 << page_size_in_bits_;
		}
		int segment_count(size_t page_size) const
		{
			return page_size / segment_size();
		}

		int inc()
		{
			return ++at(0)->segment_count_;
		}
		int dec()
		{
			return --at(0)->segment_count_;
		}
		int count()
		{
			return at(0)->segment_count_;
		}
		void count(int n)
		{
			at(0)->segment_count_ = n;
		}

		void push(segment *lst[2],char *memory_start)
		{
			prev(0,memory_start);
			if(lst[0]==0) {
				next(0,memory_start);
				lst[0] = this;
				lst[1] = this;
				return;
			}
			next(lst[0],memory_start);
			lst[0]->prev(this,memory_start);
			lst[0] = this;
		}

		void erase(segment *lst[2],char *memory_start)
		{
			if(lst[0]==this)
				lst[0]=next(memory_start);
			if(lst[1]==this)
				lst[1]=prev(memory_start);
			if(next_not_null_) {
				next(memory_start)->prev(prev(memory_start),memory_start);
			}
			if(prev_not_null_) {
				prev(memory_start)->next(next(memory_start),memory_start);
			}
		}

		segment *at(int pos)
		{
			int offset = pos - segment_offset_;
			void *newp = ((char *)(this)) + offset * segment_size();
			return static_cast<segment *>(newp);
		}

		segment *next(char *memory_start) const
		{
			if(!next_not_null_)
				return 0;
			unsigned long long p=next_msb_;
			p<<=32;
			p+=next_lsb_;
			return (segment *)(memory_start + p); 
		}
		segment *prev(char *memory_start) const
		{
			if(!prev_not_null_)
				return 0;
			unsigned long long p=prev_msb_;
			p<<=32;
			p+=prev_lsb_;
			return reinterpret_cast<segment *>(memory_start + p); 
		}
		void next(segment *p,char *memory_start)
		{
			if(p==0) {
				next_not_null_ = 0;
				return;
			}
			unsigned long long off = reinterpret_cast<char *>(p) - memory_start;
			next_msb_ = off >> 32;
			next_lsb_ = off & 0xFFFFFFFFu;
			next_not_null_ = 1;
		}
		void prev(segment *p,char *memory_start)
		{
			if(p==0) {
				prev_not_null_ = 0;
				return;
			}
			unsigned long long off = reinterpret_cast<char *>(p) - memory_start;
			prev_msb_ = off >> 32;
			prev_lsb_ = off & 0xFFFFFFFFu;
			prev_not_null_ = 1;
		}
	private:
		uint8_t page_size_in_bits_;
		uint8_t segment_count_;
		uint8_t segment_offset_;
		uint8_t next_not_null_ : 1;
		uint8_t prev_not_null_ : 1;
		uint8_t reserved_  : 6;

		uint32_t next_lsb_; 	// 0 - 5
		uint32_t prev_lsb_; 	// 0 - 5
		uint16_t next_msb_;
		uint16_t prev_msb_;
	};
	
	struct page {
		page(size_t p) 
		{
			memset(this,0,sizeof(*this));
			pages = p;
		}
		uint8_t zero; // always zero
		size_t pages;
	};

#ifdef TEST_ALLOCATOR	
public:
	void print_stats()
	{
		std::cout << "Slab" << std::endl;
		unsigned long long total_free = 0;
		for(int i=0;i<slab_bits;i++) {
			int count = 0;
			for(segment *p=all[i][0];p;p=p->next(memory_start))
				count++;
			std::cout << std::setw(8) << count << ' ' << std::setw(8) << (1ul<<(min_bits + i)) << std::endl;
			total_free += (1ul<<(min_bits + i)) * count ;
		}
		int free_super_pages = 0;
		int free_pages = 0;
		int max_block = 0;
		int last_free = 0;
		std::cout << std::setw(16) << "Free Slab" << std::setw(16) << total_free << std::endl;
		for(size_t i=0;i<map_size;i++) {
			if(map[i]==0) {
				free_super_pages++;
				last_free ++;
				if(last_free > max_block)
					max_block = last_free;
			}
			else {
				last_free=0;
			}
			unsigned long long v=map[i];
			for(int i=0;i<64;i++) {
				if((v & 1) == 0) { 
					free_pages++;
					total_free += page_size;
				}
				v>>=1;
			}
		}
		std::cout << std::setw(16) << "Pages" << std::setw(16) << free_pages * page_size << std::endl;
		std::cout << std::setw(16) << "Super Pages" <<  std::setw(16) << free_super_pages * (64 * page_size) << std::endl;
		std::cout << std::setw(16) << "Max Free"  << std::setw(16) << (64 * page_size * max_block) << std::endl;
		std::cout << std::setw(16) << "Total Free" << std::setw(16) << total_free << std::endl;
		std::cout << std::setw(16) << "Inital" << std::setw(16) << super_page_size * map_size << std::endl;
	}
	private:
#endif

	int get_optimal_size_and_bits(size_t &n)
	{
		n = (((n + (align - 1)) >> (align_bits)) + 1) << align_bits;
		if(n >= super_page_size) {
			n=(n + super_page_size - 1) / super_page_size * super_page_size;
			return -1;
		}
		else {
			int i = get_bits(n);
			n = size_t(1) << i;
			if( n>= page_size)
				return -1;
			return i;
			/*
			for(i=0;i<int(sizeof(n) * 8);i++) {
				if( (1ull << i) >= n)
					break;
			}
			n = 1ull << i;
			if(n >= page_size)
				return -1;
			return i;*/
		}
	}

	static void *make_offset(void *p)
	{
		return static_cast<char *>(p) + align;
	}

	static void *remove_offset(void *p)
	{
		return static_cast<char *>(p) - align;
	}

	void *slab_alloc(size_t n)
	{
		int bits = get_optimal_size_and_bits(n);
		if(bits == -1) {
			void *p=page_alloc(n / page_size);
			if(!p)
				return 0;
			new(p) page(n/page_size);
			return make_offset(p);
		}

		segment **lst = all[bits - min_bits];

		if(lst[0]) {
			segment *p = lst[0];
			p->erase(lst,memory_start);
			p->dec();
			return make_offset(p);
		}
		else {
			void *p=page_alloc(1);
			if(!p)
				return 0;
			segment *s = new(p) segment(bits,0);
			int count = s->segment_count(page_size);
			s->count(count-1);
			for(int i=1;i<count;i++) {
				new(s->at(i)) segment(bits,i);
				s->at(i)->push(lst,memory_start);
			}
			return make_offset(p);
		}
	}

	void slab_free(void *p)
	{
		if(!p)
			return;
		p=remove_offset(p);
		if(*static_cast<uint8_t *>(p)==0) {
			page *pg= static_cast<page *>(p);
			page_free(pg,pg->pages);
			return;
		}
		segment *s = static_cast<segment *>(p);
		segment **lst = all[s->bits() - min_bits];
		s->push(lst,memory_start);
		s=s->at(0);
		if(s->inc() == s->segment_count(page_size)) {
			int sc = s->segment_count(page_size);
			for(int i=0;i<sc;i++) {
				s->at(i)->erase(lst,memory_start);
			}
			page_free(s,1);
		}
	}

	#ifdef TEST_ALLOCATOR
	void ptest_free()
	{	
		for(int i=0;i<slab_bits;i++) {
			assert(all[i][0]==0);
			assert(all[i][1]==0);
		}
		for(size_t i = 0 ;i<map_size;i++) {
			assert(map[i]==0);
		}
	}
	#endif

	static int m2(unsigned long long v)
	{
		static unsigned long long const m1 = 0xAAAAAAAAAAAAAAAAULL;
		static unsigned long long const m2 = 0x5555555555555555ULL;

		unsigned long long first_bit_set = v & m1;
		unsigned long long second_bit_set = v & m2;
		unsigned long long first_or_second_set = first_bit_set | (second_bit_set << 1);
		
		return first_or_second_set !=0xAAAAAAAAAAAAAAAAULL;
	}

	static int m4(unsigned long long v)
	{

		//(1 or 2) ( 3 or 4 )
		// x 0 x 0
		// x 
	    
		static unsigned long long const m1 = 0xAAAAAAAAAAAAAAAAull;
		static unsigned long long const m2 = 0x5555555555555555ull;

		unsigned long long b1 = m1 & v;
		unsigned long long b2 = m2 & v;
		unsigned long long b3 = b1 | (b2 << 1);

		static unsigned long long const m3 = 0x8888888888888888ull;
		static unsigned long long const m4 = 0x2222222222222222ull;

		unsigned long long b4 = b3 & m3;
		unsigned long long b5 = b3 & m4;

		return (b4 | (b5 << 2))!=m3;

	}

	static int m8(unsigned long long v)
	{
		static unsigned long long const m1 = 0xAAAAAAAAAAAAAAAAull;
		static unsigned long long const m2 = 0x5555555555555555ull;

		unsigned long long b1 = m1 & v;
		unsigned long long b2 = m2 & v;
		unsigned long long b3 = b1 | (b2 << 1);

		static unsigned long long const m3 = 0x8888888888888888ull;
		static unsigned long long const m4 = 0x2222222222222222ull;

		unsigned long long b4 = b3 & m3;
		unsigned long long b5 = b3 & m4;
		unsigned long long b6 = (b4 | (b5 << 2));
		
		static unsigned long long const m5 = 0x8080808080808080ull;
		static unsigned long long const m6 = 0x0808080808080808ull;
		
		unsigned long long b7 = b6 & m5;
		unsigned long long b8 = b6 & m6;

		return (b7 | (b8 << 4))!=m5;
	}

	static int m16(unsigned long long v)
	{
		
		static unsigned long long m1 = 0xFFFFull << 0;
		static unsigned long long m2 = 0xFFFFull << 16;
		static unsigned long long m3 = 0xFFFFull << 32;
		static unsigned long long m4 = 0xFFFFull << 48;
		return (v & m1)==0 || (v & m2)==0 || (v & m3)==0 || (v & m4)==0;
	}

	static int m32(unsigned long long v)
	{
		
		static unsigned long long m1 = 0xFFFFFFFFull << 0;
		static unsigned long long m2 = m1 << 32;
		return (v & m1)==0 || (v & m2)==0;
	}

	/*

	static int get_bits_old(size_t n)
	{
		int i;
		for(i=0;i<int(sizeof(n) * 8);i++) {
			if( (1ull << i) >= n)
				break;
		}
		return i;
	}
	*/
	static int get_bits(size_t n)
	{
		int xh=sizeof(n)*8,xl=0,x;
		while(xh>xl+1) {
			x = (xh + xl)/2;
			if( (1ull<<x) >= n)
				xh = x;
			else 
				xl = x;
		}
		if((1ull << xl)>=n)
			return xl;
		else
			return xh;
	}


	static int find_free_bits(unsigned long long u,int n)
	{
		int p=0;
		while(u & ((1ull << n)-1)) {
			u>>=n;
			p+=n;
		}
		return p;
	}

	void *page_alloc(size_t pages)
	{
		int bit = get_bits(pages);
		size_t i=0;
		switch(bit) {
		case 0:
			for(i=0;i<map_size;i++) {
				if(map[i]!=~0ull) 
					break;
			}
			break;
		case 1:
			for(i=0;i<map_size;i++) {
				if(m2(map[i]))
					break;
			}
			break;
		case 2:
			for(i=0;i<map_size;i++) {
				if(m4(map[i]))
					break;
			}
			break;
		case 3:
			for(i=0;i<map_size;i++) {
				if(m8(map[i]))
					break;
			}
			break;
		case 4:
			for(i=0;i<map_size;i++) {
				if(m16(map[i]))
					break;
			}
			break;
		case 5:
			for(i=0;i<map_size;i++) {
				if(m32(map[i]))
					break;
			}
			break;
		}
		switch(bit) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			{
				if(i >= map_size)
					return 0;
				int bits_needed = 1 << bit;
				int pos = find_free_bits(map[i],bits_needed);
				unsigned long long bmask = (1ull << bits_needed)-1;
				map[i] |= (bmask << pos);
				return memory_start + page_size * (64 * i + pos);	
			}
		default:
			{
				size_t n = pages / 64;
				bool found = false;
				if(n==1) {
					for(i=0;i<map_size;i++) {
						if(map[i]==0) {
							found=true;
							break;
						}
					}
				}
				else {
					size_t n1=n-1;
					for(i=0;!found && i+n1<map_size;) {
						unsigned long long p2=map[i+n1];
						if(p2!=0) {
							i+=n;
							continue;
						}
						unsigned long long p1=map[i];
						if(p1!=0) {
							size_t k = n / 2;
							while(k>0 && map[i+k]==0) {
								k/=2;
							}
							i+=k+1;
							continue;
						}
						found = true;
						for(size_t j=1;j<n-1;j++) {
							if(map[i+j]) {
								i+=j+1;
								found = false;
								break;
							}
						}
					}
				}
				if(!found)
					return 0;
				for(size_t j=0;j<n;j++) {
					#ifdef CHECK
					assert(map[i+j]==0);
					#endif
					map[i+j]=~0ull;
				}
				return memory_start + super_page_size * i;
			}
		}
	}

	void page_free(void *p,size_t n)
	{
		int bit = get_bits(n);
		size_t i = (static_cast<char*>(p) - memory_start) / super_page_size;
		if(bit<=5) {
			{
				int pos = (static_cast<char *>(p) - memory_start) % (super_page_size) / page_size;
				int bits_needed = 1 << bit;
				unsigned long long bmask = (1ull << bits_needed)-1;
				map[i] &= ~(bmask << pos);
			}
		}
		else {
			n /= 64;
			while(n>0) {
				map[i]=0;
				n--;
				i++;
			}
		}
	}



}; // memory_manager 

} // details
class shmem_control : public booster::noncopyable{
public:
	shmem_control(size_t size) :
		size_(size),
		region_(mmap_anonymous(size)),
		memory_(0)
	{
		memory_ = new (region_) details::memory_manager(size_);
	}
	~shmem_control()
	{
		::munmap(region_,size_);
	}
	inline size_t size()
	{
		return size_;
	}
	inline size_t available() 
	{ 
		mutex::guard g(lock_);
		return memory_->get_free_memory();

	}
	inline void *malloc(size_t s)
	{
		mutex::guard g(lock_);
		return memory_->malloc(s);
	}
	inline void free(void *p) 
	{
		mutex::guard g(lock_);
		return memory_->free(p);
	}

	#ifdef TEST_ALLOCATOR
	void test_free()
	{
		memory_->test_free();
	}
	void print_stats()
	{
		memory_->print_stats();
	}
	#endif
private:
	mutex lock_;	
	size_t size_;
	void *region_;
	details::memory_manager *memory_;
};

template<typename T,shmem_control *&mm>
class shmem_allocator : public basic_allocator<shmem_allocator<T,mm>, T > {
public :

	typedef basic_allocator<shmem_allocator<T,mm>, T > super;
	template<typename U>
	struct rebind {
		typedef shmem_allocator<U,mm> other;
	};

	template<typename U>
	shmem_allocator (const shmem_allocator< U, mm > &) :
		super()
	{
	}

	shmem_allocator (const shmem_allocator &) :
		super()
	{
	}
	shmem_allocator()
	{
	}

	void *malloc(size_t n) const
	{
		return mm->malloc(n);
	}
	void free(void *p) const
	{
		return mm->free(p);
	}

};



} //
} //


#endif
