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
#include <cppcms/cppcms_error.h>
#include <sstream>
#include "test.h"


std::string value(std::ostringstream &s)
{
	std::string res = s.str();
	s.str("");
	return res;
}

int main()
{
	try {
		cppcms::url_mapper m;
		m.assign("foo","/foo");
		m.assign("foo","/foo/{1}");
		m.assign("foo","/foo/{1}/{2}");
		m.assign("foo","/foo/{1}/{2}/{3}");
		m.assign("foo","/foo/{1}/{2}/{3}/{4}");
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
	catch(std::exception const &e) {
		std::cerr << "Fail" << e.what() << std::endl;
		return 1;
	}
	std::cout << "ok" << std::endl;
	return 0;
}

