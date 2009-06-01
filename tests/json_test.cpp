#include "json.h"
#include <iostream>
#include <stdexcept>
using namespace cppcms;
using namespace std;

#define TEST(x) if(!(x)) throw std::runtime_error("Failed" #x); else cout<<#x " Ok"<<endl

void test1(json::value const &v)
{
	TEST(v("foo.bar").integer()==10);
	TEST(v("foo.pee").is_null());
	TEST(v("foo.lst")[3].boolean()==true);
	TEST(v("x").real()==13.5);
	TEST(v["x"].real()==13.5);
	TEST(v("y").str()=="אב");
	TEST(v.type()==json::is_object);
}
void test2(json::value &v)
{
	TEST(v("foo.bar").integer()==10);
	TEST(v("foo.pee").is_null());
	TEST(v("foo.lst")[3].boolean()==true);
	TEST(v("x").real()==13.5);
	TEST(v["x"].real()==13.5);
	TEST(v("y").wstr()==L"אב");
	TEST(v("y").str()=="א");
	TEST(v.type()==json::is_object);
}

char const *jsn_str=
		"{ \"x\" : 10 , \"o\" : { \"test\" : [ 10,20,true ], \"post\" : 13.01 }, \"yes\" : \"\\u05d0א\" }";


int main()
{
	try {
		json::value v;
		cout<<v<<endl;
		v("foo.bar")=10;
		v("foo.lst")[0]="yes";
		v("foo.lst")[1]=15;
		v["foo"]["lst"][3]=true;
		v["x"]=13.5;
		v["y"]="אב";
		cout<<v<<endl;
		cout<<v.save(json::readable)<<endl;
		test1(v);
		json::value v2=v;
		TEST(v2==v);
		test2(v);
		cout<<v.save(json::readable)<<endl;
		TEST(v2!=v);
		json::value v_1;
		v_1.load(jsn_str);
		json::value v_2;
		v_2["x"]=10;
		v_2("o.test")[0]=10;
		v_2("o.test")[1]=20;
		v_2("o.test")[2]=true;
		v_2("o.post")=13.01;
		v_2("yes")=L"אא";
		TEST(v_2==v_1);
		TEST(v_1["yes"].wstr()==L"אא");
		TEST(v_2["yes"].str()=="אא" );
	}
	catch(std::exception const &e)
	{
		cerr<<e.what()<<endl;
		return 1;
	}
	return 0;
}

