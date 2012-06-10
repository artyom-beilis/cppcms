///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_HASH_MAP_H
#define CPPCMS_IMPL_HASH_MAP_H

#include <new>
#include <functional>
#include <vector>
#include <assert.h>

#include <cppcms/cstdint.h>
#include <booster/iterator/iterator_facade.h>
namespace cppcms {
namespace impl {
	
template<typename T>
struct string_hash {
public:
	typedef uint32_t state_type;
	static const state_type initial_state = 0;
	static state_type update_state(state_type value,char c)
	{
		value = (value << 4) + static_cast<unsigned char>(c);
		uint32_t high = (value & 0xF0000000U);
		if(high!=0)
		value = (value ^ (high >> 24)) ^ high;
		return value;
	}

	size_t operator()(T const &v) const
	{
		state_type st = initial_state;
		for(typename T::const_iterator p=v.begin();p!=v.end();++p) {
			st = update_state(st,*p);
		}
		return st;
	}
};
	
namespace details {


template<typename T>
class intrusive_list
{
public:
	intrusive_list() : begin(0), end(0)
	{
	}

	void push_back(T *p)
	{
		p->next = 0;
		p->prev = end;
		if(end)
			end->next = p;
		end = p;

		if(begin == 0)
			begin = p;
	}
	void push_front(T *p)
	{
		p->next = begin;
		p->prev = 0;
		if(begin)
			begin->prev = p;
		begin = p;
		if(end == 0)
			end = p;
	}
	void erase(T *p)
	{
		if(p->prev) {
			p->prev->next = p->next;
		}
		if(p->next) {
			p->next->prev = p->prev;
		}
		if(begin == p)
			begin = p->next;
		if(end == p)
			end = p->prev;
		p->next = p->prev = 0;
	}

	void insert_after(T *p,T *after_me)
	{
		if(!after_me->next) {
			push_back(p);
			return;
		}
		p->prev = after_me;
		p->next = after_me->next;
		if(after_me->next)
			after_me->next->prev = p;
		after_me->next = p;
	}
	void insert_before(T *p,T *before_me)
	{
		if(!before_me->prev) {
			push_front(p);
			return;
		}
		p->next = before_me;
		p->prev = before_me->prev;
		if(before_me->prev)
			before_me->prev->next = p;
		before_me->prev = p;
	}
	void swap(intrusive_list &other)
	{
		std::swap(other.begin,begin);
		std::swap(other.end,end);
	}

	T *begin,*end;
};


template<	typename Key,
		typename Value,
		typename Hash,
		typename Alloc= std::allocator<std::pair<Key,Value> >
		>
class basic_map
{
public:
	
	basic_map() : size_(0)
	{
	}
	
	~basic_map()
	{
		clear();
	}

	typedef std::pair<const Key,Value> value_type;

	struct container {
		container() : val(), next(0),prev(0) {}
		container(value_type const &v) : val(v), next(0),prev(0) {}
		value_type val;
		size_t hash;
		container *next;
		container *prev;
	};

	typedef container *iterator;
	typedef typename Alloc::template rebind<container>::other container_alloc;

	typedef std::pair<iterator,iterator> range_type;
	typedef typename Alloc::template rebind<range_type>::other vec_allocator;

	iterator erase(iterator p)
	{
		if(p==0)
			return 0;
		range_type &r = get(p->val.first);
		if(r.first == r.second) {
			assert(p == r.first);
			r.first = r.second = 0;
		}
		else if(r.first == p)
			r.first = p->next;
		else if(r.second == p)
			r.second = p->prev;
		iterator next = p->next;
		list_.erase(p);
		size_ --;
		destroy(p);
		return next;
	}
	size_t size() const
	{
		return size_;
	}

	iterator find(Key const &k)
	{
		if(hash_.empty())
			return 0;
		range_type &range=get(k);
		return find_in_range(range,k);
	}


	std::pair<iterator,bool> insert(value_type const &entry)
	{
		std::pair<iterator,bool> r(0,false);
		rehash_if_needed();
		range_type &range=get(entry.first);
		iterator p = find_in_range(range,entry.first);
		if(p) {
			r.first = p;
			return r;
		}
		p = allocate(entry);
		if(range.second == 0) {
			list_.push_back(p);
			range.first = range.second = p;
		}
		else {
			list_.insert_after(p,range.second);
			range.second = p;
		}

		r.first = p;
		r.second  = true;
		size_ ++;
		return r;
	}

	void clear()
	{
		iterator p=list_.begin;
		if(size_ / 4 >= hash_.size()) {
			for(size_t i=0;i<hash_.size();i++) {
				hash_[i].first = 0;
				hash_[i].second = 0;
			}
			while(p) {
				iterator del = p;
				p=p->next;
				del->prev = 0;
				del->next = 0;
				destroy(del);
			}
		}
		else {
			while(p) {
				iterator del = p;
				p=p->next;
				del->prev = 0;
				del->next = 0;
				range_type &r=get(del->val.first);
				r.first = r.second = 0;
				destroy(del);
			}
		}
		list_.begin = list_.end = 0;
		size_ = 0;
	}

	iterator begin()
	{
		return list_.begin;
	}
	iterator end()
	{
		return 0;
	}

#ifdef TEST_MAP
	void check()
	{
		size_t count = 0;
		for(iterator p=list_.begin;p!=0;p=p->next,count++) {
			range_type &r = get(p->val.first);
			if(r.first == r.second) {
				TEST(r.first==p);
			}
			bool found = false;
			iterator p2;
			for(p2=r.first;p2!=r.second;p2=p2->next) {
				if(p2 == p) {
					found = true;
					break;
				}
			}
			if(!found)
				TEST(p2 == p);
			if(p->next == 0) {
				TEST(p == list_.end);
			}
		}
		TEST(size_ == count);
		count = 0;
		for(size_t i=0;i<hash_.size();i++) {
			for(iterator p=hash_[i].first;p;p=p->next) {
				count++;
				Hash h;
				TEST(h(p->val.first) % hash_.size() == i);
				if(p==hash_[i].second)
					break;
			}
		}
		TEST(size_ == count);
	}
#endif


private:
	iterator find_in_range(range_type &r,Key const &k)
	{
		for(iterator p=r.first;p!=0;p=p->next) {
			if(p->val.first == k)
				return p;
		}
		return 0;
	}
	range_type &get(Key const &k)
	{
		Hash hf;
		size_t h = hf(k) % hash_.size();
		return hash_[h];
	}

	size_t next_size()
	{
		return (1+size_)*2;
	}
	void rehash(size_t new_size)
	{
		basic_map tmp;
		tmp.hash_.resize(new_size,range_type(0,0));
		while(list_.begin) {
			iterator p=list_.begin;
			list_.erase(p);
			range_type &r = tmp.get(p->val.first);
			if(r.first == 0) {
				tmp.list_.push_back(p);
				r.first = r.second = p;
			}
			else {
				tmp.list_.insert_after(p,r.second);
				r.second = p;
			}
		}
		list_.swap(tmp.list_);
		hash_.swap(tmp.hash_);
		tmp.hash_.clear();
	}
	void rehash_if_needed()
	{
		size_t table_size = hash_.size();
		if(size_ + 1 >= table_size) {
			rehash(next_size());
		}
	}

	iterator allocate(value_type const &v)
	{
		container_alloc al;
		iterator p = al.allocate(1);
		try {
			new (p) container(v);
		}
		catch(...) {
			al.deallocate(p,1);
			throw;
		}
		return p;
	}
	iterator allocate()
	{
		container_alloc al;
		iterator p = al.allocate(1);
		try {
			new (p) container();
		}
		catch(...) {
			al.deallocate(p);
			throw;
		}
		return p;
	}

	void destroy(iterator p)
	{
		container_alloc al;
		p->~container();
		al.deallocate(p,1);
	}

	std::vector<range_type, vec_allocator>  hash_;
	intrusive_list<container> list_;
	size_t size_;
};


} // details

template<	typename Key,
		typename Value,
		typename Hash,
		typename Alloc= std::allocator<std::pair<Key,Value> > >
class hash_map {
	typedef details::basic_map<Key,Value,Hash,Alloc> impl_type;
	typedef typename impl_type::iterator impl_iterator;
public:
	typedef std::pair<const Key,Value> value_type;

#ifdef TEST_MAP
        void check()
        {
            impl_.check();
        }
#endif

        ~hash_map()
        {
        	clear();
        }

	class iterator :
		public booster::iterator_facade<
				iterator,
				value_type,
				booster::bidirectional_traversal_tag>
	{
	public:
		iterator(typename impl_type::iterator p = 0) : p_(p) {}

		value_type &dereference() const
		{
			return p_->val;
		}
		void increment()
		{
			p_=p_->next;
		}
		void decrement()
		{
			p_=p_->prev;
		}
		bool equal(iterator const &other) const
		{
			return p_==other.p_;
		}
	private:
		friend class hash_map;
		typename impl_type::iterator p_;
	};

	iterator begin()
	{
		return iterator(impl_.begin());
	}
	iterator end()
	{
		return iterator(0);
	}

	iterator find(Key const &k)
	{
		return iterator(impl_.find(k));
	}

	std::pair<iterator,bool> insert(value_type const &v)
	{
		std::pair<impl_iterator,bool> r = impl_.insert(v);
		return std::pair<iterator,bool>(iterator(r.first),r.second);
	}
	iterator erase(iterator p)
	{
		return iterator(impl_.erase(p.p_));
	}
	void clear()
	{
		impl_.clear();
	}
	
	size_t size() const
	{
            return impl_.size();
        }

private:
	impl_type impl_;
};




} // impl
} // cppcms


#endif
