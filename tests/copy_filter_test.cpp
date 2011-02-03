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
#include "test.h"
#include <iostream>
#include <cppcms/copy_filter.h>


int main()
{
	try {
		std::cout << "- Normal flow" << std::endl;
		std::ostringstream ss;
		ss << "a";
		{
			cppcms::copy_filter flt(ss);
			ss << "bc";
			TEST(flt.detach()=="bc");
			ss << "d";
		}
		ss << "e";
		TEST(ss.str()=="abcde");
		std::cout << "- With Flush" << std::endl;
		ss.clear();
		ss.str("");
		ss << "a";
		{
			cppcms::copy_filter flt(ss);
			ss << "b";
			ss << std::flush;
			ss << "c";
			TEST(flt.detach()=="bc");
		}
		ss << "d";
		TEST(ss.str()=="abcd");
		std::cout << "- With throw and flush" << std::endl;
		ss.clear();
		ss.str("");
		ss << "a";
		try {
			cppcms::copy_filter flt(ss);
			ss << "b";
			ss << std::flush;
			throw int(1);
		}
		catch(int x){}
		ss << "c";
		TEST(ss.str()=="abc");
		std::cout << "- With throw and no flush" << std::endl;
		ss.clear();
		ss.str("");
		ss << "a";
		try {
			cppcms::copy_filter flt(ss);
			ss << "b";
			throw int(1);
		}
		catch(int x){}
		ss << "c";
		TEST(ss.str()=="ac");

	}
	catch(std::exception const &e) {
		std::cerr << "Fail:" << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;

}
