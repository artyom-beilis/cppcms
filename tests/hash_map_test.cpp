#define TEST_MAP
#include "test.h"
#include <iostream>
#include "hash_map.h"
#include "basic_allocator.h"
#include <stdlib.h>

struct hasher {
	size_t operator()(std::string const &v) const
	{
		size_t n = 0;
		for(size_t i=0;i<v.size();i++)
			n+=v[i] * 172 + 13;
		return n;
	}
};

#define INTERNAL() do { check_line = __LINE__; h.check(); } while(0)

struct simple {
	int x;
	simple *next,*prev;
	simple(int v) : x(v),next(0),prev(0) {}
};

static int alloc_count = 0;
static int total_allocs = 0;
static int max_allocs = 0;

template<typename T>
struct my_alloc : public cppcms::impl::basic_allocator<my_alloc<T>,T> {

	typedef cppcms::impl::basic_allocator<my_alloc<T>,T> super;
	template<typename U> my_alloc(my_alloc<U> const &) : super() {}

	my_alloc(){}

	template<typename U>
	struct rebind {
		typedef my_alloc<U> other;
	};


	void *malloc(size_t n) const
	{
		alloc_count++;
		total_allocs++;
		if(alloc_count > max_allocs)
			max_allocs+=alloc_count;
		return ::malloc(n);
	}
	void free(void *p) const
	{
		if(p)
			alloc_count --;
		::free(p);
	}
};


void test_list()
{
	std::cout << "Test list" << std::endl;

	{
		simple a(1),b(2),c(3),d(3);
		cppcms::impl::details::intrusive_list<simple> lst;
		TEST(lst.begin == lst.end && lst.begin == 0);
		lst.push_front(&a);
		TEST(a.next == a.prev && a.next == 0);
		TEST(lst.begin == lst.end && lst.begin == &a);
		lst.erase(&a);
		TEST(a.next == a.prev && a.next == 0);
		TEST(lst.begin == lst.end && lst.begin == 0);
		lst.push_back(&a);
		TEST(a.next == a.prev && a.next == 0);
		TEST(lst.begin == lst.end && lst.begin == &a);
		lst.push_back(&b);
		TEST(lst.begin == &a);
		TEST(lst.end == &b);
		TEST(a.next == &b);
		TEST(b.prev == &a);
		TEST(a.prev == 0);
		TEST(b.next == 0);
		lst.push_front(&c);
		TEST(lst.begin == &c);
		TEST(c.next == &a && a.prev == &c);
		lst.insert_after(&d,&c);
		TEST(lst.begin == &c);
		TEST(c.next == &d);
		TEST(d.prev == &c);
		TEST(d.next == &a);
		TEST(a.prev == &d);
		// c,d,a,b
		lst.erase(&c);
		TEST(lst.begin == &d);
		TEST(d.prev == 0);
		TEST(c.next == c.prev && c.next == 0);
		lst.erase(&a);
		TEST(d.next == &b);
		TEST(b.prev == &d);
		TEST(lst.begin == &d);
		TEST(lst.end == &b);
		// d,b
		lst.insert_before(&a,&b);
		TEST(d.next == &a);
		TEST(b.prev == &a);
		TEST(a.next == &b);
		TEST(a.prev == &d);
		// d,a,b
	}


}

static int i=-1;
static int check_line = 0;

template<typename Alloc>
void test_hasher()
{
	typedef cppcms::impl::hash_map<std::string,int,hasher,cppcms::impl::details::are_equal,Alloc> hasher_type;
	typedef typename hasher_type::iterator iterator;
	hasher_type h;
	typedef std::pair<std::string,int> vt;
	std::cout << "Empty" << std::endl;
	INTERNAL();
	std::cout << "Insert" << std::endl;
	for(i=0;i<100;i++) {
		std::ostringstream ss;
		ss << i;
		std::pair<iterator,bool> pr = h.insert(vt(ss.str(),i));
		INTERNAL();
		TEST(pr.second);
		TEST(pr.first->first == ss.str());
		TEST(pr.first->second == i);
	}
	std::cout << "Check Inserted" << std::endl;
	{
		TEST(h.size()==100);
		std::pair<iterator,bool> pr = h.insert(vt("10",32));
		INTERNAL();
		TEST(!pr.second);
		TEST(pr.first->first == "10");
		TEST(pr.first->second == 10);
		h.erase(pr.first);
		INTERNAL();
		TEST(h.size()==99);
		pr = h.insert(vt("10",32));
		INTERNAL();
		TEST(h.size()==100);
		TEST(pr.second);
		TEST(pr.first->first == "10");
		TEST(pr.first->second == 32);
		pr.first->second = 10;
	}
	std::cout << "Check All Inserted" << std::endl;
	for(i=0;i<100;i++) {
		std::ostringstream ss;
		ss << i;
		iterator p = h.find(ss.str());
		INTERNAL();
		TEST(p!=h.end());
		TEST(p->first == ss.str());
		TEST(p->second == i);
	}
	std::vector<int> marked(100,0);
	for(iterator p=h.begin();p!=h.end();++p) {
		marked[atoi(p->first.c_str())]++;
	}
	for(i=0;i<100;i++)
		TEST(marked[i]==1);
	std::cout << "Check Erase" << std::endl;
	size_t size = 100;
	for(iterator p=h.begin();p!=h.end();p=h.begin()) {
		h.erase(p);
		INTERNAL();
		size --;
		TEST(h.size() == size);
	}
	h.clear();
	INTERNAL();
	std::cout << "Check All Inserted again" << std::endl;
	for(i=0;i<100;i++) {
		std::ostringstream ss;
		ss << i;
		h.insert(vt(ss.str(),i));
		INTERNAL();
	}
	h.clear();
	INTERNAL();
	std::cout << "Check Partial Inserted again" << std::endl;
	for(i=0;i<10;i++) {
		std::ostringstream ss;
		ss << i;
		h.insert(vt(ss.str(),i));
		INTERNAL();
	}
	h.clear();
	INTERNAL();
}


int main()
{
	try {
		test_list();
		std::cout << "Standard Allocator" << std::endl;
		test_hasher< std::allocator<std::pair<std::string,int> > >();
		std::cout << "Custom Allocator" << std::endl;
		test_hasher< my_alloc<std::pair<std::string,int> > >();
		// for debuggins
		// std::cout << alloc_count << " " << total_allocs << " " << max_allocs << std::endl;
		//
		TEST(alloc_count == 0);
	}
	catch(std::exception const &e) {
		std::cerr << "Error: " << e.what() << " i=" << i << " check_line=" << check_line<< std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
}
