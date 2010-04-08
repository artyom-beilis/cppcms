#include "json.h"
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
		THROWS(parse("true"));	
		THROWS(parse("false"));	
		THROWS(parse("10"));	
		THROWS(parse("\"hello\""));	
		THROWS(parse("null"));
		TEST(parse("[true]")[0]==json::value(true));	
		TEST(parse("[false]")[0]==json::value(false));	
		TEST(parse("[10]")[0]==json::value(10));	
		TEST(parse("[\"hello\"]")[0]==json::value("hello"));	
		TEST(parse("[null]")[0]==json::null());
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
		THROWS(parse("[\"\\ud834\"]"));
		THROWS(parse("[\"\\udd1e\"]"));
		THROWS(parse("[\"\\ud834 \\udd1e\"]"));
		TEST(parse("[\"\\u05d0\\ud834\\udd1e x\"]")[0]=="א\xf0\x9d\x84\x9e x");
		THROWS(parse("[\"\xFF\xFF\"]")); // Invalid UTF-8
		TEST(parse("[\"\\u05d0 x\"]")[0]=="א x"); // Correct read of 4 bytes
		THROWS(parse("[\"\\u05dx x\"]")); // Correct read of 4 bytes
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

