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
#include <cppcms/json.h>
#include "test.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace cppcms;
using namespace std;

char const *jsn_str=
		"{ \"t\" : [{},{},{},{ \"x\":1},[]],\"x\" : { \"o\" : { \"test\" : [ 10,20,true ], \"post\" : 13.01 }, \"yes\" : \"\\u05d0א\" }}";

class parsing_error : public std::runtime_error {
public:
	parsing_error(std::string s) : std::runtime_error(s) {}
};

#define THROWS(X) do{ try { X; }catch(cppcms::json::bad_value_cast const &e){}\
	catch(parsing_error const &e){}\
	catch(...){\
	std::ostringstream tmp;	\
	tmp << __FILE__ << " " << __LINE__ << " "#X " not throwed"; \
	throw std::runtime_error(tmp.str()); } }while(0)


json::value Parse(std::string s,int line)
{
	std::istringstream ss(s);
	json::value v;
	if(!v.load(ss,true)) {
		std::ostringstream tmp;
		tmp << "Parsing error of " << s << " in line " << line;
		throw parsing_error(tmp.str());
	}
	return v;
}

std::string format(json::value const &v)
{
	std::ostringstream ss;
	ss<<v;
	return ss.str();
}

#define parse(X) Parse(X,__LINE__)

int main()
{
	try {
		json::value v;
		json::value const &vc=v;
		TEST(v.type()==json::is_undefined);
		v=10;
		TEST(v.type()==json::is_number);
		TEST(v.number()==10);
		TEST(v.get_value<int>()==10);
		TEST(v.get_value<double>()==10);
		THROWS(v.get_value<std::string>());
		v="test";
		TEST(v.type()==json::is_string);
		TEST(v.str()=="test");
		v=true;
		TEST(v.type()==json::is_boolean);
		TEST(v.boolean()==true);
		v=json::null();
		TEST(v.type()==json::is_null);
		v=json::array();
		TEST(v.type()==json::is_array);
		v=json::object();
		TEST(v.type()==json::is_object);
		TEST(v.find("x")==json::value());
		TEST(v.type("x")==json::is_undefined);
		TEST(v.get<std::string>("x","y")=="y");
		THROWS(v.get<std::string>("x"));
		THROWS(v.at("x"));
		v["x"]=10;
		TEST(v.find("x")==json::value(10));
		TEST(v.at("x")==json::value(10));
		TEST(v.type("x")==json::is_number);
		TEST(v.get<std::string>("x","y")=="y");
		THROWS(v.get<std::string>("x"));
		v.set("x.y.z",10);
		TEST(v["x"]["y"]["z"].number()==10);
		TEST(v.get<int>("x.y.z")==10);
		TEST(parse("[]")==json::array());	
		TEST(parse("{}")==json::object());	
		TEST(parse("true")==json::value(true));	
		TEST(parse("false")==json::value(false));	
		TEST(parse("10")==json::value(10));	
		TEST(parse("\"hello\"")==json::value("hello"));	
		TEST(parse("null")==json::null());
		char const *s=
			"{ \"t\" : [{},{},{},{ \"x\":1},[]],\"x\" : { \"o\" : { \"test\" : [ 10,20,true ], \"post\" : 13.01 }, \"yes\" : \"\\u05d0א\" }}";
		v=parse(s);
		TEST(v.type("t")==json::is_array);
		TEST(v["t"].array().size()==5);
		TEST(v["t"][0]==json::object());
		TEST(v["t"][1]==json::object());
		TEST(v["t"][2]==json::object());
		TEST(v["t"][4]==json::array());
		TEST(v["t"][3]["x"].number()==1);
		TEST(v.type("x")==json::is_object);
		TEST(v.get<std::string>("x.yes")=="אא");
		// Test correct handing of surrogates
		THROWS(parse("\"\\ud834\""));
		THROWS(parse("\"\\udd1e\""));
		THROWS(parse("\"\\ud834 \\udd1e\""));
		TEST(parse("\"\\u05d0\\ud834\\udd1e x\"")=="א\xf0\x9d\x84\x9e x");
		THROWS(parse("\"\xFF\xFF\"")); // Invalid UTF-8
		TEST(parse("\"\\u05d0 x\"")=="א x"); // Correct read of 4 bytes
		THROWS(parse("\"\\u05dx x\"")); // Correct read of 4 bytes
		TEST(parse("[//Hello\n]")==json::array());
		TEST(format("test")=="\"test\"");
		TEST(format(10)=="10");
		TEST(format(true)=="true");
		TEST(format(false)=="false");
		TEST(format(json::null())=="null");
		TEST(format(json::object())=="{}");
		TEST(format(json::array())=="[]");
		v=json::value();
		v["x"]="yes";
		TEST(vc["x"].str()=="yes");
		THROWS(vc["y"]);
		THROWS(vc[1]);
		v[2]="yes";
		TEST(v[0]==json::null());
		TEST(v[1]==json::null());
		TEST(vc[2].str()=="yes");
		TEST(v[2]=="yes");
		TEST(v[3]==json::null());
		THROWS(vc[4]);
		THROWS(vc["x"]);
		v=json::value();
		v["x"]=10;
		v["y"][1]="test";
		v["y"][2]=json::array();
		TEST(format(v)=="{\"x\":10,\"y\":[null,\"test\",[]]}");

	}
	catch(std::exception const &e)
	{
		cerr<<"Failed:"<<e.what()<<endl;
		return 1;
	}
	std::cout << "Passed" << std::endl;
	return 0;
}

