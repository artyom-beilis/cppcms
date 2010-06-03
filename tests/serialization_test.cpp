#include <cppcms/serialization.h>

#include "test.h"


struct foo {
	double t,v;
};

struct test2 : public cppcms::serializable {
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
	
	void serialize(cppcms::archive &a)
	{
		a & cppcms::as_pod(f) & x & y;
	}
};


struct test1  : public cppcms::serializable {
	void init()
	{
		str="Hello";
		x=11;
		v.push_back(10);
		v.push_back(20);
		mapping[10].init();
		mapping[20].init();
		p.reset(new test2);
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
	std::map<int,test2> mapping;
	booster::shared_ptr<test2> p;

	void serialize(cppcms::archive &a)
	{
		a & str & x & v & mapping;
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
		Pointer x,y(new int(15));
		a & x;
		a.mode(cppcms::archive::load_from_archive);
		a & y;
		TEST(y.get()==0);
		TEST(a.eof());

	}
}



int main()
{
	try {
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
			std::multimap<std::string,std::vector<int> > x,y;
			x.insert(std::pair<std::string,std::vector<int> >("foo",std::vector<int>(10,20)));
			a & x;
			a.mode(cppcms::archive::load_from_archive);
			a & y;
			TEST(x==y);
			TEST(a.eof());
		}
		test_ptr<booster::shared_ptr<int> >();
		test_ptr<booster::hold_ptr<int> >();
		test_ptr<booster::copy_ptr<int> >();
		test_ptr<booster::clone_ptr<int> >();
		test_ptr<std::auto_ptr<int> >();

		std::string tmp;
		{
			test2 t;
			t.init();
			{
				test2 const &tt=t;
				cppcms::serialization_traits<cppcms::serializable_base>::save(tt,tmp);
			}
		}
		{
			test2 t;
			cppcms::serialization_traits<cppcms::serializable_base>::load(tmp,t);
			t.check();
		}


	}
	catch(std::exception const &e) {
		std::cerr << "Fail: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
}
