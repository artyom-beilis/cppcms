///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/serialization.h>

#include "test.h"


struct foo {
	double t,v;
};

struct test2_base {
	foo f;
	int x,y;

	void init()
	{
		f.t=100;
		f.v=20;
		x=65;
		y=10;
	}
	void check()
	{
		TEST(f.t==100);
		TEST(f.v==20);
		TEST(x==65);
		TEST(y==10);
	}

};

struct test2 : public test2_base, public cppcms::serializable 
{	
	void serialize(cppcms::archive &a)
	{
		a & cppcms::as_pod(f) & x & y;
	}
};

struct test2c : public test2_base, public cppcms::serializable_base 
{	
	void save(cppcms::archive &a) const
	{
		a << cppcms::as_pod(f) << x << y;
	}
	void load(cppcms::archive &a)
	{
		a >> cppcms::as_pod(f) >> x >> y;
	}
};

template<typename Test2>
struct test1_base {
	void init()
	{
		str="Hello";
		x=11;
		v.push_back(10);
		v.push_back(20);
		mapping[10].init();
		mapping[20].init();
		p.reset(new Test2);
		p->init();
	}
	void check()
	{
		TEST(str=="Hello" && x == 11 && v[0]==10 && v[1]==20 && v.size()==2);
		TEST(mapping.size()==2);
		mapping[10].check();
		mapping[20].check();
		TEST(mapping.size()==2);
		TEST(p);
		p->check();
	}

	std::string str;
	int x;
	std::vector<int> v;
	std::map<int,Test2> mapping;
	booster::shared_ptr<Test2> p;
};

struct test1  : public test1_base<test2>, public cppcms::serializable 
{

	void serialize(cppcms::archive &a)
	{
		a & str & x & v & mapping & p;
	}
};

struct test1c  : public test1_base<test2c>, public cppcms::serializable_base 
{

	void save(cppcms::archive &a) const
	{
		a << str << x << v << mapping << p;
	}

	void load(cppcms::archive &a)
	{
		a >> str >> x >> v >> mapping >> p;
	}
};


template<typename Pointer>
void test_ptr()
{
	{
		cppcms::archive a;
		Pointer x(new int(15)),y;
		a & x;
		a.mode(cppcms::archive::load_from_archive);
		a & y;
		TEST(*x==*y && x.get()!=y.get());
		TEST(a.eof());

	}
	{
		cppcms::archive a;
		Pointer const x(new int(15));
		Pointer y;
		a << x;
		a.mode(cppcms::archive::load_from_archive);
		a >> y;
		TEST(*x==*y && x.get()!=y.get());
		TEST(a.eof());

	}
	{
		cppcms::archive a;
		Pointer x,y(new int(15));
		a & x;
		a.mode(cppcms::archive::load_from_archive);
		a & y;
		TEST(y.get()==0);
		TEST(a.eof());
	}
	{
		cppcms::archive a;
		Pointer const x;
		Pointer y(new int(15));
		a << x;
		a.mode(cppcms::archive::load_from_archive);
		a >> y;
		TEST(y.get()==0);
		TEST(a.eof());
	}

}

template<typename T>
T const &const_ref(T &x)
{
	return x;
}

int main()
{
	try {
		std::cout << "Testing POD" << std::endl;
		{
			cppcms::archive a;
			unsigned int x=0xDEADBEEF;
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			unsigned int y = 0xFFFFFFFF;
			a & y;
			TEST(y==0xDEADBEEF);
			TEST(a.eof());
			a.reset();
			TEST(a.next_chunk_size() == sizeof(int));
		}
		{
			cppcms::archive a;
			const unsigned int x=0xDEADBEEF;
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			unsigned int y = 0xFFFFFFFF;
			a >> y;
			TEST(y==0xDEADBEEF);
			TEST(a.eof());
			a.reset();
			TEST(a.next_chunk_size() == sizeof(int));
		}

		std::cout << "Testing String" << std::endl;
		{
			cppcms::archive a;
			std::string x="Hello World";
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			std::string y;
			a & y;
			TEST(y==x);
			TEST(a.eof());
			a.reset();
			TEST(a.next_chunk_size() == x.size());
		}
		{
			cppcms::archive a;
			std::string const x="Hello World";
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			std::string y;
			a >> y;
			TEST(y==x);
			TEST(a.eof());
			a.reset();
			TEST(a.next_chunk_size() == x.size());
		}

		std::cout << "Testing array" << std::endl;
		{
			cppcms::archive a;
			std::string x[2]={"hello","world"};
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			std::string y[2];
			a >> y;
			TEST(x[0]==y[0] && x[1]==y[1]);
			TEST(a.eof());
		}
		{
			cppcms::archive a;
			std::string const x[2]={"hello","world"};
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			std::string y[2];
			a >> y;
			TEST(x[0]==y[0] && x[1]==y[1]);
			TEST(a.eof());
		}

		std::cout << "Testing POD array" << std::endl;
		{
			cppcms::archive a;
			int x[2]={10,20};
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			TEST(a.next_chunk_size()==sizeof(int)*2);
			int y[2]={0,0};
			a & y;
			TEST(x[0]==y[0] && x[1]==y[1]);
			TEST(a.eof());
		}
		{
			cppcms::archive a;
			int const x[2]={10,20};
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			TEST(a.next_chunk_size()==sizeof(int)*2);
			int y[2]={0,0};
			a >> y;
			TEST(x[0]==y[0] && x[1]==y[1]);
			TEST(a.eof());
		}

		std::cout << "Testing POD vector" << std::endl;
		{
			cppcms::archive a;
			std::vector<int> x;
			x.push_back(10);
			x.push_back(20);
			x.push_back(30);
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			TEST(a.next_chunk_size()==sizeof(int)*3);
			std::vector<int> y;
			a & y;
			TEST(x==y);
			TEST(a.eof());
		}
		{
			cppcms::archive a;
			std::vector<int> xr;
			xr.push_back(10);
			xr.push_back(20);
			xr.push_back(30);
			std::vector<int> const &x=xr;
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			TEST(a.next_chunk_size()==sizeof(int)*3);
			std::vector<int> y;
			a >> y;
			TEST(x==y);
			TEST(a.eof());
		}

		std::cout << "Testing list" << std::endl;
		{
			cppcms::archive a;
			std::list<int> x;
			x.push_back(10);
			x.push_back(20);
			x.push_back(30);
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			std::list<int> y;
			a & y;
			TEST(x==y);
			TEST(a.eof());
		}
		{
			cppcms::archive a;
			std::list<int> xr;
			xr.push_back(10);
			xr.push_back(20);
			xr.push_back(30);
			std::list<int> const &x=xr;
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			std::list<int> y;
			a >> y;
			TEST(x==y);
			TEST(a.eof());
		}

		std::cout << "Testing compund" << std::endl;
		{
			cppcms::archive a;
			std::vector<std::string> x;
			x.push_back("a");
			x.push_back("XX");
			x.push_back("tttt");
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			std::vector<std::string> y;
			a & y;
			TEST(x==y);
			TEST(a.eof());
		}
		{
			cppcms::archive a;
			std::vector<std::string> xr;
			xr.push_back("a");
			xr.push_back("XX");
			xr.push_back("tttt");
			std::vector<std::string> const &x=xr;
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			std::vector<std::string> y;
			a >> y;
			TEST(x==y);
			TEST(a.eof());
		}

		
		std::cout << "Testing set" << std::endl;
		{
			cppcms::archive a;
			std::set<std::string> x;
			x.insert("a");
			x.insert("XX");
			x.insert("tttt");
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			std::set<std::string> y;
			a & y;
			TEST(x==y);
			TEST(a.eof());
		}

		{
			cppcms::archive a;
			std::set<std::string> xr;
			xr.insert("a");
			xr.insert("XX");
			xr.insert("tttt");
			std::set<std::string> const &x=xr;
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			std::set<std::string> y;
			a >> y;
			TEST(x==y);
			TEST(a.eof());
		}

		
		std::cout << "Testing map" << std::endl;
		{
			cppcms::archive a;
			std::map<std::string,std::vector<int> > x;
			x["a"].push_back(10);
			x["a"].push_back(20);
			x["b"].push_back(40);
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			std::map<std::string,std::vector<int> > y;
			a & y;
			TEST(x==y);
			TEST(a.eof());
		}
		{
			cppcms::archive a;
			std::map<std::string,std::vector<int> > xr;
			xr["a"].push_back(10);
			xr["a"].push_back(20);
			xr["b"].push_back(40);
			std::map<std::string,std::vector<int> > const &x=xr;
			a << x;
			a.mode(cppcms::archive::load_from_archive);
			std::map<std::string,std::vector<int> > y;
			a >> y;
			TEST(x==y);
			TEST(a.eof());
		}

		
		std::cout << "Testing multimap" << std::endl;
		{
			cppcms::archive a;
			std::multimap<std::string,std::vector<int> > x,y;
			x.insert(std::pair<std::string,std::vector<int> >("foo",std::vector<int>(10,20)));
			a << const_ref(x);
			a.mode(cppcms::archive::load_from_archive);
			a >> y;
			TEST(x==y);
			TEST(a.eof());
		}
		std::cout << "Testing smart pointer" << std::endl;
	
		test_ptr<booster::shared_ptr<int> >();
		test_ptr<booster::hold_ptr<int> >();
		test_ptr<booster::copy_ptr<int> >();
		test_ptr<booster::clone_ptr<int> >();
		test_ptr<std::auto_ptr<int> >();

		std::cout << "Testing object serialization" << std::endl;
		{
			std::string tmp;
			{
				test1 t;
				t.init();
				{
					test1 const &tt=t;
					cppcms::serialization_traits<test1>::save(tt,tmp);
				}
			}
			{
				test1 t;
				cppcms::serialization_traits<test1>::load(tmp,t);
				t.check();
			}
		}

		{
			std::string tmp;
			{
				test1c t;
				t.init();
				{
					test1c const &tt=t;
					cppcms::serialization_traits<test1c>::save(tt,tmp);
				}
			}
			{
				test1c t;
				cppcms::serialization_traits<test1c>::load(tmp,t);
				t.check();
			}
		}


	}
	catch(std::exception const &e) {
		std::cerr << "Fail: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
}
