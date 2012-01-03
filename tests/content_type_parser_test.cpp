///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
//#define DEBUG_MULTIPART_PARSER
#include <cppcms/http_content_type.h>
#include <iostream>
#include "test.h"
#include <string.h>


int main()
{
	try {
		using cppcms::http::content_type;
		{
			content_type t("hello/world");
			TEST(t.type()=="hello");
			TEST(t.subtype()=="world");
			TEST(t.media_type()=="hello/world");
		}
		{
			content_type t("Hello/WORLD; chaRset=UTF-8");
			TEST(t.type()=="hello");
			TEST(t.subtype()=="world");
			TEST(t.media_type()=="hello/world");
			TEST(t.parameter_is_set("charset"));
			TEST(!t.parameter_is_set("xx"));
			TEST(t.charset()=="UTF-8");
		}
		{
			content_type t("hello/world ; chaRset=\"UTF-8\"");
			TEST(t.type()=="hello");
			TEST(t.subtype()=="world");
			TEST(t.media_type()=="hello/world");
			TEST(t.charset()=="UTF-8");
			TEST(t.parameter_is_set("charset"));
		}
		{
			content_type t("hello/world; foo=1; bar=2; beep=\"x\\\"\"; zee=hello");
			TEST(t.type()=="hello");
			TEST(t.subtype()=="world");
			TEST(t.media_type()=="hello/world");
			TEST(t.parameter_by_key("foo")=="1");
			TEST(t.parameter_by_key("bar")=="2");
			TEST(t.parameter_by_key("beep")=="x\"");
			TEST(t.parameter_by_key("zee")=="hello");
			TEST(t.parameter_by_key("x")=="");
		}
		{
			content_type t("foo");
			TEST(t.type()=="");
			TEST(t.subtype()=="");
			TEST(t.media_type()=="");
		}
		{
			content_type t("foo/");
			TEST(t.type()=="");
			TEST(t.subtype()=="");
			TEST(t.media_type()=="");
		}
		{
			content_type t("/foo");
			TEST(t.type()=="");
			TEST(t.subtype()=="");
			TEST(t.media_type()=="");
		}
		{
			content_type t("foo/bar;; charset=UTF-8");
			TEST(t.type()=="foo");
			TEST(t.subtype()=="bar");
			TEST(t.media_type()=="foo/bar");
			TEST(t.charset()=="");
		}
		{
			content_type t("foo/bar; charset=\"UTF-8");
			TEST(t.type()=="foo");
			TEST(t.subtype()=="bar");
			TEST(t.media_type()=="foo/bar");
			TEST(t.charset()=="");
		}
		
	}
	catch(std::exception const &e) {
		std::cerr << "Fail: " <<e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
