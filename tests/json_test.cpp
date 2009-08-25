#include "json.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace cppcms;
using namespace std;

#define TEST(x) if(!(x)) throw std::runtime_error("Failed" #x); else cout<<#x " Ok"<<endl


char const *jsn_str=
		"{ \"t\" : [{},{},{},{ \"x\":1},[]],\"x\" : { \"o\" : { \"test\" : [ 10,20,true ], \"post\" : 13.01 }, \"yes\" : \"\\u05d0א\" }}";


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
		cout<<"-------------"<<endl;
		cout<<v<<endl;
		cout<<"-------------"<<endl;

		std::stringstream res(v.save(json::readable));
		cout<<res.str()<<endl;

		json::value v2;
		int line_no = -1 ;
		cout<<"-------------"<<endl;
		if(v2.load(res,true,&line_no)) {
			v2.save(cout,json::readable);
			cout<<"-------------"<<endl;
		}
		else {
			cout<<line_no<<endl;
			cout<<"-------------"<<endl;
		}
		std::stringstream ss(jsn_str);
		if(v2.load(ss,true,&line_no)) {
			v2.save(cout,json::readable);
			cout<<"-------------"<<endl;
		}
		else {
			cout<<line_no<<endl;
			cout<<"-------------"<<endl;
		}
		
	}
	catch(std::exception const &e)
	{
		cerr<<"Catched:"<<e.what()<<endl;
		return 1;
	}
	return 0;
}

