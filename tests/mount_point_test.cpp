#include <cppcms/mount_point.h>
#include <booster/regex.h>
#include <iostream>
#include "test.h"

std::pair<bool,std::string> res(char const *p)
{
	std::pair<bool,std::string> r(false,"");
	if(p) {
		r.first = true;
		r.second = p;
	}
	return r;
}

void test()
{
	using cppcms::mount_point;
	{
		mount_point mp("/test.*");
		TEST(mp.match("www.google.com","/test","/foo")==res("/foo"));
		TEST(mp.match("www.google.com","/test/bar","/foo")==res("/foo"));
		TEST(mp.match("www.google.com","x/test","/foo")==res(0));
	}
	{
		mount_point mp("/test(.*)",1);
		TEST(mp.match("www.google.com","/baz","/test")==res(""));
		TEST(mp.match("www.google.com","/baz","/test/")==res("/"));
		TEST(mp.match("www.google.com","/baz","x/test/")==res(0));
	}
	{
		mount_point mp(mount_point::match_path_info,"/foo");
		TEST(mp.match("www.google.com","/foo","/test")==res("/test"));
		TEST(mp.match("www.google.com","z/foo","/test")==res(0));
	}
	{
		mount_point mp(mount_point::match_script_name,"/foo");
		TEST(mp.match("www.google.com","/bar","/foo")==res("/bar"));
		TEST(mp.match("www.google.com","/bar","/baz")==res(0));
	}
	{
		mount_point mp("/foo","/test(.*)",1);
		TEST(mp.match("www.google.com","/foo","/test")==res(""));
		TEST(mp.match("www.google.com","/foo","/test/")==res("/"));
		TEST(mp.match("www.google.com","/foo","x/test/")==res(0));
		TEST(mp.match("www.google.com","/bar","/test")==res(0));
		TEST(mp.match("www.google.com","/bar/","/test")==res(0));
	}
	{
		mount_point mp(mount_point::match_path_info,"/baz(.*)",1);
		TEST(mp.match("www.google.com","/foo.php","/baz/xx")==res("/xx"));
		TEST(mp.match("www.google.com","/foo.phpx","x/baz")==res(0));
	}
	{
		mount_point mp(mount_point::match_path_info,"/test","/foo.*",0);
		TEST(mp.match("www.google.com","/test","/foo")==res("/foo"));
		TEST(mp.match("www.google.com","/test","/fooz")==res("/fooz"));
		TEST(mp.match("www.google.com","/testx","/fooz")==res(0));
		TEST(mp.match("www.google.com","/test","/fo")==res(0));
	}
	{
		mount_point mp(mount_point::match_script_name,"(.*)\\.php",1);
		TEST(mp.match("www.google.com","/foo.php","/test")==res("/foo"));
		TEST(mp.match("www.google.com","/foo.phpx","/test")==res(0));
	}
	{
		mount_point mp(mount_point::match_script_name,"/test","(.*)\\.php",1);
		TEST(mp.match("www.google.com","/foo.php","/test")==res("/foo"));
		TEST(mp.match("www.google.com","/foo.php","/test1")==res(0));
		TEST(mp.match("www.google.com","/foo.php","/test1")==res(0));
	}
	{
		mount_point mp(	mount_point::match_script_name,
				booster::regex("(.*\\.)?cppcms.com"),
				booster::regex("/.*.php"),
				booster::regex("/bar"),
				0);
		TEST(mp.match("artyom.cppcms.com","/foo.php","/bar")==res("/foo.php"));
		TEST(mp.match("google.com","/foo.php","/bar")==res(0));
		TEST(mp.match("artyom.cppcms.com","/foo.aspx","/bar")==res(0));
		TEST(mp.match("artyom.cppcms.com","/foo.php","/baz")==res(0));
	}
	{
		mount_point mp(	mount_point::match_path_info,
				booster::regex("(.*\\.)?cppcms.com"),
				booster::regex("/test"),
				booster::regex("/(.*)"),
				0);
		TEST(mp.match("artyom.cppcms.com","/test","/bar")==res("/bar"));
		TEST(mp.match("artyom.cppcms.com","/testx","/bar")==res(0));
		TEST(mp.match("artyom.cppcms.com","/test","")==res(0));
	}
	{
		mount_point mp(	mount_point::match_path_info,
				booster::regex(),
				booster::regex(),
				booster::regex(),
				0);
		TEST(mp.match("artyom.cppcms.com","/test","/bar")==res("/bar"));
	}
	{
		mount_point mp(	mount_point::match_script_name,
				booster::regex(),
				booster::regex(),
				booster::regex(),
				0);
		TEST(mp.match("artyom.cppcms.com","/test","/bar")==res("/test"));
	}
}

int main()
{
	try {
		test();
	}
	catch(std::exception const &e) {
		std::cerr << "Fail:"<<e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
