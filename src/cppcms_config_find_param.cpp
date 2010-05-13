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
#include "json.h"
#include <iostream>
#include <fstream>

int main(int argc,char **argv)
{
	using namespace cppcms;
	if(argc!=3)
		return 1;
	std::ifstream in(argv[2]);
	if(!in)
		return 1;
	
	json::value v;
	if(!v.load(in,true))
		return 1;
	std::string path=argv[1];
	try {
		std::cout<<v.get<double>(path);
		return 0;
	}
	catch(json::bad_value_cast const  &e) {}
	try {
		std::cout<<v.get<std::string>(path);
		return 0;
	}
	catch(json::bad_value_cast const  &e) {}
	try {
		std::vector<std::string> vs=v.get<std::vector<std::string> >(path);
		std::string sep="";
		for(unsigned i=0;i<vs.size();i++) {
			std::cout<<sep<<vs[i];
			sep=" ";
		}
		return 0;
	}
	catch(json::bad_value_cast const  &e) {}
	return 1;
}
