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
	TEST(v("y").str()=="True Name");
	TEST(v.type()==json::is_object);
}
void test2(json::value &v)
{
	TEST(v("foo.bar").integer()==10);
	TEST(v("foo.pee").is_null());
	TEST(v("foo.lst")[3].boolean()==true);
	TEST(v("x").real()==13.5);
	TEST(v["x"].real()==13.5);
	TEST(v("y").str()=="True Name");
	TEST(v.type()==json::is_object);
}
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
		v["y"]="True Name";
		cout<<v<<endl;
		cout<<v.save(json::readable)<<endl;
		test1(v);
		json::value v2=v;
		TEST(v2==v);
		test2(v);
		cout<<v.save(json::readable)<<endl;
		TEST(v2!=v);
	}
	catch(std::exception const &e)
	{
		cerr<<e.what()<<endl;
		return 1;
	}
	return 0;
}

