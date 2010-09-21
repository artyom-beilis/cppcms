//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <booster/regex.h>
#include "test.h"

#include <iostream>

bool search(std::string r,std::string t)
{
	booster::regex re(r);
	bool v1 =  re.search(t.c_str(),t.c_str()+t.size());
	std::vector<std::pair<int,int> > m;
	bool v2 =  re.search(t.c_str(),t.c_str()+t.size(),m);
	TEST(v1==v2);
	return v1;
}

bool match(std::string r,std::string t)
{
	booster::regex re(r);
	std::vector<std::pair<int,int> > m;
	bool v1 = re.match(t.c_str(),t.c_str()+t.size());
	bool v2 = re.match(t.c_str(),t.c_str()+t.size(),m);
	TEST(v1==v2);
	return v1;
}

template<typename Match,typename Other>
void check_less(Match const &a,Other b)
{
	TEST(a<b);
	TEST(a<=b);
	TEST(!(a>b));
	TEST(!(a>=b));
	TEST(b>a);
	TEST(b>=a);
	TEST(!(b<a));
	TEST(!(b<=a));
	TEST(a!=b);
	TEST(b!=a);
	TEST(!(a==b));
	TEST(!(b==a));
}

template<typename Match,typename Other>
void check_equal(Match const &a,Other b)
{
	TEST(a==b);
	TEST(b==a);
	TEST(!(a!=b));
	TEST(!(b!=a));
}

template<typename Param,typename MatchType>
void test_match(Param str)
{
	MatchType cm;
	TEST(booster::regex_match(str,cm,booster::regex("(a)(a)(x)?(b)")));
	TEST(cm.size()==5);
	TEST(cm[0].matched && cm[1].matched && cm[1].matched && !cm[3].matched && cm[4].matched);

	TEST(cm[0]=="aab");
	TEST(cm[1]=="a");
	TEST(cm[2]=="a");
	TEST(cm[3]!="a");
	TEST(cm[3]=="");
	TEST(cm[4]=="b");

	check_equal(cm[1],cm[2]);
	check_less(cm[1],cm[4]);
	check_less(cm[3],cm[1]);

	check_equal(cm[1],"a");
	check_less(cm[1],"b");
	check_less("",cm[1]);

	check_equal(cm[1],std::string("a"));
	check_less(cm[1],std::string("b"));
	check_less(std::string(""),cm[1]);

	TEST(cm[1]+cm[4]=="ab");
	TEST("a"+cm[4]=="ab");
	TEST(cm[1]+"b"=="ab");
	TEST(std::string("a")+cm[4]=="ab");
	TEST(cm[1]+std::string("b")=="ab");
}

void re_match(bool result,std::string re,std::string pat,std::string key="",int pos=0)
{
	TEST(booster::regex_match(pat,booster::regex(re))==result);
	TEST(booster::regex_match(pat.c_str(),pat.c_str()+pat.size(),booster::regex(re))==result);
	TEST(booster::regex_match(pat.c_str(),booster::regex(re))==result);

	if(result) {
		{
			booster::smatch sm;
			TEST(booster::regex_match(pat,sm,booster::regex(re)));
			TEST(sm[pos]==key);
		}
		{
			booster::cmatch cm;
			TEST(booster::regex_match(pat.c_str(),pat.c_str()+pat.size(),cm,booster::regex(re)));
			TEST(cm[pos]==key);
		}
		{
			booster::cmatch cm;
			TEST(booster::regex_match(pat.c_str(),cm,booster::regex(re)));
			TEST(cm[pos]==key);
		}
	}
}

void re_search(bool result,std::string re,std::string pat,std::string key="",int pos=0)
{
	TEST(booster::regex_search(pat,booster::regex(re))==result);
	TEST(booster::regex_search(pat.c_str(),pat.c_str()+pat.size(),booster::regex(re))==result);
	TEST(booster::regex_search(pat.c_str(),booster::regex(re))==result);

	if(result) {
		{
			booster::smatch sm;
			TEST(booster::regex_search(pat,sm,booster::regex(re)));
			TEST(sm[pos]==key);
		}
		{
			booster::cmatch cm;
			TEST(booster::regex_search(pat.c_str(),pat.c_str()+pat.size(),cm,booster::regex(re)));
			TEST(cm[pos]==key);
		}
		{
			booster::cmatch cm;
			TEST(booster::regex_search(pat.c_str(),cm,booster::regex(re)));
			TEST(cm[pos]==key);
		}
	}
}




int main()
{
	try {
		{
			std::cout << "Testing search/match" << std::endl;
			TEST(search("foo","foo"));
			TEST(search("foo","foobar"));
			TEST(search("foo","zeefoo"));
			TEST(!search("foo","bar"));

			TEST(match("foo","foo"));
			TEST(!match("foo","zeefoo"));
			TEST(!match("foo","foobar"));
			TEST(match("(foo)","foo"));
			TEST(!match("(foo)","zeefoo"));
			TEST(!match("(foo)","foobar"));
			TEST(match("((((((((((foo))))))))))","foo"));
			TEST(!match("((((((((((foo))))))))))","foox"));
			TEST(!match("((((((((((foo))))))))))","xfoo"));
			
			std::cout << "Testing match_result" << std::endl;
			test_match<std::string,booster::smatch>("aab");
			test_match<char const *,booster::cmatch>("aab");

			re_match(true,"foo","foo","foo",0);
			re_match(false,"foo","Foo");
			re_search(false,"foo","Foo");
			re_match(false,"f.o","foobar");
			re_search(true,"f.o","foobar","foo",0);

			std::cout << "Testing regex match/search" << std::endl;
			booster::regex re("foo");
			booster::cmatch cm;
			TEST(booster::regex_search("foo",cm,re));
			TEST(cm.prefix()=="" && cm.suffix()=="");
			
			TEST(booster::regex_search("foobee",cm,re));
			TEST(cm.prefix()=="" && cm.suffix()=="bee");
			
			TEST(booster::regex_search("zfoo",cm,re));
			TEST(cm.prefix()=="z" && cm.suffix()=="");

			TEST(booster::regex_search("zfoobee",cm,re));
			TEST(cm.prefix()=="z" && cm.suffix()=="bee");

			{
				std::cout << "Testing regex ctor/operator=" << std::endl;
				booster::regex r;
				r.assign("(t(x))");
				TEST(r.str()=="(t(x))");
				TEST(r.mark_count() == 2);
				booster::regex rc(r);
				TEST(booster::regex_match("tx",rc));
				TEST(!booster::regex_match("ty",rc));

				re = r;

				TEST(booster::regex_match("tx",re));
				TEST(!booster::regex_match("ty",re));

			}

		}
	}
	catch(std::exception const &e) {
		std::cerr << "Fail: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
