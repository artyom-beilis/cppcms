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
#include "function.h"
#include "test.h"
#include <iostream>

bool foov_called;
void foov() { 
	foov_called=true;
}
bool fooi_called;
int fooi()
{
	fooi_called=true;
	return 10;
}
bool fooid_called;

int fooid(double const &v)
{
	TEST(v==10.0);
	fooid_called = true;
	return 0;
}

int fooii(int &x)
{
	return x++;
}

void reset()
{
	foov_called = fooi_called = fooid_called = false;
}


int main()
{
	using cppcms::function;
	try {
		reset();
		function<void()> f=foov;
		f();
		TEST(foov_called);
		f=fooi;
		f();
		TEST(fooi_called);
		function<int()> fi=fooi;
		TEST(fi()==10);
		TEST(!fi.empty());
		function<void(int)> fvi;
		TEST(fvi.empty());
		try {
			fvi(10);
			throw std::runtime_error("Not throws!");
		}
		catch(cppcms::bad_function_call const &e) {}
		fvi=fooid;
		TEST(!fvi.empty());
		fvi(10);
		TEST(fooid_called);
		int x=2;
		TEST(function<int(int&)>(fooii)(x)==2);
		TEST(x==3);
	}
	catch(std::exception const &e)
	{
		std::cout << "Fail"<< e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
