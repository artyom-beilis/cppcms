#include "json.h"
#include <iostream>
#include <stdexcept>
using namespace cppcms;
using namespace std;

#define TEST(x) if(!(x)) throw std::runtime_error("Failed" #x); else cout<<#x " Ok"<<endl


char const *jsn_str=
		"{ \"x\" : 10 , \"o\" : { \"test\" : [ 10,20,true ], \"post\" : 13.01 }, \"yes\" : \"\\u05d0א\" }";


int main()
{
	try {
		json::value v;
		v["foo"]["bar"]=10;
		v["y"]="Hello \x3 World";
		v["foo"]["bar"].number()+=5;
		v.at("foo.bar").number()-=1;


		v.set("foo.lst",json::array());

		cout<<v.at("foo")<<endl;

		v.at("foo.lst").array().push_back("yes");
		v.at("foo.lst").array().push_back(15);
		
		
		v.set("x",13.5);
		v["x"]="hello";
		cout<<v<<endl;

		cout<<v.save(json::readable)<<endl;
	}
	catch(std::exception const &e)
	{
		cerr<<"Catched:"<<e.what()<<endl;
		return 1;
	}
	return 0;
}

