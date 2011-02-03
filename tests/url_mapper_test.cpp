///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/url_mapper.h>
#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/json.h>
#include <cppcms/cppcms_error.h>
#include <sstream>
#include "test.h"


std::string value(std::ostringstream &s)
{
	std::string res = s.str();
	s.str("");
	return res;
}

void basic_test()
{
	cppcms::url_mapper m(0);

	std::cout << "-- Basic Mapping" << std::endl;

	m.assign("foo","/foo");
	m.assign("foo","/foo/{1}");
	m.assign("foo","/foo/{1}/{2}");
	m.assign("foo","/foo/{1}/{2}/{3}");
	m.assign("foo","/foo/{1}/{2}/{3}/{4}");
	m.assign("foo","/foo/{1}/{2}/{3}/{4}/{5}");
	m.assign("foo","/foo/{1}/{2}/{3}/{4}/{5}/{6}");
	m.assign("bar","/bar/{lang}/{1}");
	m.assign("bar","/bar/{2}/{1}");
	m.assign("test1","{1}x");
	m.assign("test2","x{1}");
	m.set_value("lang","en");
	m.root("test.com");
	TEST(m.root()=="test.com");

	std::ostringstream ss;
	m.map(ss,"foo");
	TEST(value(ss) == "test.com/foo");


	m.map(ss,"foo",1);
	TEST(value(ss) == "test.com/foo/1");
	m.map(ss,"foo",1,2);
	TEST(value(ss) == "test.com/foo/1/2");
	m.map(ss,"foo",1,2,3);
	TEST(value(ss) == "test.com/foo/1/2/3");
	m.map(ss,"foo",1,2,3,4);
	TEST(value(ss) == "test.com/foo/1/2/3/4");
	m.map(ss,"foo",1,2,3,4,5);
	TEST(value(ss) == "test.com/foo/1/2/3/4/5");
	m.map(ss,"foo",1,2,3,4,5,6);
	TEST(value(ss) == "test.com/foo/1/2/3/4/5/6");

	m.map(ss,"foo",1,"a","b",5);
	TEST(value(ss) == "test.com/foo/1/a/b/5");
	
	m.map(ss,"bar",1);
	TEST(value(ss) == "test.com/bar/en/1");
	m.map(ss,"bar",1,"ru");
	TEST(value(ss) == "test.com/bar/ru/1");
	m.root("");
	m.map(ss,"test1",10);
	TEST(value(ss) == "10x");
	m.map(ss,"test2",10);
	TEST(value(ss) == "x10");

	std::cout << "-- Testing throwing at invalid params" << std::endl;
	try {
		m.assign("x","a{");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }

	try {
		m.assign("x","a}");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }
	
	try {
		m.assign("x","a{0}");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }
	try {
		m.map(ss,"undefined");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }
	
	try {
		m.map(ss,"undefined");
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }
	
	try {
		m.map(ss,"test1",1,2);
		TEST(0);
	}
	catch(cppcms::cppcms_error const &e) {}
	catch(...) { TEST(0); }
}


struct test1 : public cppcms::application {
	test1(cppcms::service &s) : cppcms::application(s)
	{
		mapper().assign("/default");
		mapper().assign("somepath","/");
		mapper().assign("page","/{1}");
		mapper().assign("preview","/{1}/preview");
		mapper().assign("bylang","/{lang}");
		mapper().assign("byloc","/{lang}_{terr}");
		mapper().assign("byloc","/{lang}_{terr}/{1}");
	}
};

struct test2 : public cppcms::application {
	test1 bee;
	test2(cppcms::service &s) : 
		cppcms::application(s),
		bee(s)
	{
		mapper().assign("somepath","/");
		mapper().assign("page","/{1}");
		mapper().assign("preview","/{1}/preview");
		mapper().mount("bee","/bee{1}",bee);
	}
};

struct test_app : public cppcms::application {
	test1 foo,bar;
	test2 bee;

	test_app(cppcms::service &s) : 
		cppcms::application(s),
		foo(s),
		bar(s),
		bee(s)
	{ 
		add(foo);
		add(bar);
		add(bee);
		mapper().mount("foo","/foo{1}",foo);
		mapper().mount("bar","/bar{1}",bar);
		mapper().mount("foobar","/foobar{1}",bee);
		mapper().assign("somepath","/test");
		mapper().root("xx");

		bee.bee.mapper().set_value("lang","en");
		mapper().set_value("terr","US");
	}
	void test_hierarchy()
	{
		std::ostringstream ss;
		mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/test");

		foo.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/foo/");
		foo.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/foo/1");
		foo.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/foo/1/preview");

		bar.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/bar/");
		bar.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/bar/1");
		bar.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/bar/1/preview");


		bee.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/foobar/");
		bee.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/foobar/1");
		bee.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/foobar/1/preview");

		bee.bee.mapper().map(ss,"somepath");
		TEST(value(ss) == "xx/foobar/bee/");
		bee.bee.mapper().map(ss,"page",1);
		TEST(value(ss) == "xx/foobar/bee/1");
		bee.bee.mapper().map(ss,"preview",1);
		TEST(value(ss) == "xx/foobar/bee/1/preview");
		
		bee.bee.mapper().map(ss,"bylang");
		TEST(value(ss) == "xx/foobar/bee/en");

	}
	std::string u(cppcms::application &a,std::string const &param)
	{
		std::ostringstream ss;
		a.mapper().map(ss,param);
		return ss.str();
	}
	void test_mapping()
	{
		TEST(u(*this,"/somepath")=="xx/test");
		TEST(u(*this,"/foo/somepath")=="xx/foo/");
		TEST(u(*this,"/foo/")=="xx/foo/default");
		TEST(u(*this,"/foo")=="xx/foo/default");
		TEST(u(*this,"/foobar/bee/")=="xx/foobar/bee/default");
		TEST(u(*this,"/foobar/bee")=="xx/foobar/bee/default");
		TEST(u(*this,"/foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(*this,"somepath")=="xx/test");
		TEST(u(*this,"foo/somepath")=="xx/foo/");
		TEST(u(*this,"foobar/bee/somepath")=="xx/foobar/bee/");
		TEST(u(*this,"foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(bar,"/somepath")=="xx/test");
		TEST(u(bar,"/foo/somepath")=="xx/foo/");
		TEST(u(bar,"/foobar/bee/somepath")=="xx/foobar/bee/");
		TEST(u(bar,"/foobar/bee/")=="xx/foobar/bee/default");
		TEST(u(bar,"/foobar/bee")=="xx/foobar/bee/default");
		TEST(u(bar,"/foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(bee.bee,"/somepath")=="xx/test");
		TEST(u(bee.bee,"/foo/somepath")=="xx/foo/");
		TEST(u(bee.bee,"/foobar/bee/somepath")=="xx/foobar/bee/");
		TEST(u(bee.bee,"/foobar/bee/bylang")=="xx/foobar/bee/en");
		
		TEST(u(bee.bee,"somepath")=="xx/foobar/bee/");
		TEST(u(bee.bee,"bylang")=="xx/foobar/bee/en");
		
		TEST(u(bee.bee,"../somepath")=="xx/foobar/");
		TEST(u(bee.bee,"../../somepath")=="xx/test");
		TEST(u(bee.bee,"bylang")=="xx/foobar/bee/en");
		TEST(u(foo,"../foobar/bee/bylang")=="xx/foobar/bee/en");
	}
	void test_keywords()
	{
		std::ostringstream ss;
		mapper().map(ss,"/foobar/bee/bylang");
		TEST(value(ss)=="xx/foobar/bee/en");
		mapper().map(ss,"/foobar/bee/bylang;lang","ru");
		TEST(value(ss)=="xx/foobar/bee/ru");
		mapper().map(ss,"/foobar/bee/byloc;lang","ru");
		TEST(value(ss)=="xx/foobar/bee/ru_US");
		mapper().map(ss,"/foobar/bee/byloc;lang,terr","ru","RU");
		TEST(value(ss)=="xx/foobar/bee/ru_RU");
		mapper().map(ss,"/foobar/bee/byloc;terr","RU");
		TEST(value(ss)=="xx/foobar/bee/en_RU");
		mapper().map(ss,"/foobar/bee/byloc;terr,lang","UA","ru");
		TEST(value(ss)=="xx/foobar/bee/ru_UA");
		mapper().map(ss,"/foobar/bee/byloc;lang","ru","foo");
		TEST(value(ss)=="xx/foobar/bee/ru_US/foo");
		mapper().map(ss,"/foobar/bee/byloc;lang,terr","ru","RU","foo");
		TEST(value(ss)=="xx/foobar/bee/ru_RU/foo");
		mapper().map(ss,"/foobar/bee/byloc;terr","RU","foo");
		TEST(value(ss)=="xx/foobar/bee/en_RU/foo");
		mapper().map(ss,"/foobar/bee/byloc;terr,lang","UA","ru","foo");
		TEST(value(ss)=="xx/foobar/bee/ru_UA/foo");
	}
};


int main()
{
	try {
		std::cout << "- Basics" << std::endl;
		basic_test();
		
		std::cout << "- Hierarchy " << std::endl;
		cppcms::json::value cfg;
		cppcms::service srv(cfg);
		test_app app(srv);

		app.test_hierarchy();

		std::cout << "- Mappting" << std::endl;
		app.test_mapping();
		std::cout << "- Keyword substitution" << std::endl;
		app.test_keywords();

	}
	catch(std::exception const &e) {
		std::cerr << "Fail: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "ok" << std::endl;
	return 0;
}

