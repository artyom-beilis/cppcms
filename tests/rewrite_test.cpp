///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include "rewrite.h"
#include "test.h"

cppcms::json::value rule(std::string e,std::string p,bool f=true)
{
	cppcms::json::value v;
	v["regex"]=e;
	v["pattern"]=p;
	v["final"]=f;
	return v;
}

cppcms::json::value one_rule(std::string e,std::string p)
{
	cppcms::json::value v;
	v[0]["regex"]=e;
	v[0]["pattern"]=p;
	return v;
}

void test_rewrite(cppcms::json::value const &v,char const *src,char const *tgt)
{
	cppcms::impl::url_rewriter rw(v.array());
	cppcms::impl::string_pool p;
	char *tmp = strdup(src);
	if(strcmp(rw.rewrite(tmp,p),tgt)!=0) {
		std::cerr << "Rule" << v << std::endl;
		std::cerr << "Failed:[" << src << "] --> [" << tgt <<"]!=[" << tmp << "]" << std::endl;
		free(tmp);
		throw std::runtime_error("Incorrect rewrite");
	}
	free(tmp);
}


void run_test()
{
	{
		cppcms::json::value r;
		r[0]=rule("/foo/(.*)","/bar/$1");
		r[1]=rule("/bar/(.*)","/bee/$1");
		test_rewrite(r,"/foo/x","/bar/x");
		test_rewrite(r,"/food/x","/food/x");
		test_rewrite(r,"/p/foo/x","/p/foo/x");
		test_rewrite(r,"/bar/abc","/bee/abc");
		r[0]["final"]=false;
		test_rewrite(r,"/foo/x","/bee/x");
	}
	{
		test_rewrite(one_rule("a(\\d+)b(\\d+)c(\\d+)d","a$2b$3c$1d"),"a5b6c7d","a6b7c5d");
		test_rewrite(one_rule(".*","$$$0$$$1"),"a","$a$");
		test_rewrite(one_rule(".*","$$-$$"),"a","$-$");
		test_rewrite(one_rule(".*","$0-$$"),"a","a-$");
		test_rewrite(one_rule(".*","$$-$0"),"a","$-a");
	}
}

int main()
{
	try {
		run_test();
	}
	catch(std::exception const &e) {
		std::cerr << "Failed: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
