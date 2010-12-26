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
#include <cppcms/xss.h>
#include "test.h"
#include <iostream>
#include <string.h>

void test_rules()
{
	std::cout << "- Testing rules" << std::endl;
	using namespace cppcms::xss;
	
	{
		std::cout << "-- Testing basic properties" << std::endl;
		rules r;
		TEST(r.html() == rules::xhtml_input);
		r.html(rules::html_input);
		TEST(r.html() == rules::html_input);
		r.html(rules::xhtml_input);
		TEST(r.html() == rules::xhtml_input);
		TEST(!r.comments_allowed());
		r.comments_allowed(true);
		TEST(r.comments_allowed());
		TEST(!r.numeric_entities_allowed());
		r.numeric_entities_allowed(true);
		TEST(r.numeric_entities_allowed());
	}
	{
		std::cout << "-- Testing entities" << std::endl;
		rules r;
		TEST(r.valid_entity(details::c_string("amp")));
		TEST(!r.valid_entity(details::c_string("Amp")));
		TEST(!r.valid_entity(details::c_string("foo")));
		r.html(rules::html_input);
		TEST(!r.valid_entity(details::c_string("Amp")));
		r.add_entity("foo");
		TEST(r.valid_entity(details::c_string("foo")));
		TEST(!r.valid_entity(details::c_string("bar")));

	}
	{
		std::cout <<"-- Testing tags" << std::endl;
		std::cout <<"--- XHTML" << std::endl;
		{
			rules r;
			TEST(r.valid_tag("a")==rules::invalid_tag);
			TEST(r.valid_tag("hr")==rules::invalid_tag);
			r.add_tag("a",rules::any_tag);
			TEST(r.valid_tag("A")==rules::invalid_tag);
			TEST(r.valid_tag("a")==rules::any_tag);
			r.add_tag("hr",rules::stand_alone);
			TEST(r.valid_tag("hr")==rules::stand_alone);
			TEST(r.valid_tag("HR")==rules::invalid_tag);
			TEST(r.valid_tag("Hr")==rules::invalid_tag);
		}
		std::cout <<"--- HTML" << std::endl;
		{
			rules r;
			r.html(rules::html_input);
			TEST(r.valid_tag("a")==rules::invalid_tag);
			TEST(r.valid_tag("hr")==rules::invalid_tag);
			r.add_tag("a",rules::any_tag);
			TEST(r.valid_tag("A")==rules::any_tag);
			TEST(r.valid_tag("a")==rules::any_tag);
			r.add_tag("hr",rules::stand_alone);
			TEST(r.valid_tag("hr")==rules::stand_alone);
			TEST(r.valid_tag("HR")==rules::stand_alone);
			TEST(r.valid_tag("Hr")==rules::stand_alone);
		}
	}
	{
		std::cout <<"-- Testing properties" << std::endl;
		std::cout <<"--- XHTML" << std::endl;
		{
			rules r;
			r.add_tag("a",rules::opening_and_closing);
			r.add_tag("b",rules::opening_and_closing);
			TEST(!r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(!r.valid_boolean_property("a","src"));
			r.add_property("a","src",booster::regex(".*"));
			TEST(r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(!r.valid_property("b","src","javascript:alert('XSS')"));
			r.add_property("a","src",booster::regex("(http|ftp|https)://.*"));
			TEST(!r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(!r.valid_property("a","src","javascript:alert('XSS') http://foo"));
			TEST(r.valid_property("a","src","http://foo"));
			TEST(!r.valid_property("a","Src","http://foo"));
			r.add_boolean_property("a","disabled");
			TEST(r.valid_property("a","disabled","disabled"));
			TEST(!r.valid_boolean_property("a","disabled"));
			r.add_integer_property("a","border");
			TEST(r.valid_property("a","border","0"));
			TEST(r.valid_property("a","border","3415423452452454235234523"));
			TEST(r.valid_property("a","border","-3415423452452454235234523"));
			TEST(!r.valid_property("a","border","-"));
			TEST(!r.valid_property("a","border","1.3"));
			TEST(!r.valid_property("a","border",""));

		}
		std::cout <<"--- HTML" << std::endl;
		{
			rules r;
			r.html(rules::html_input);
			r.add_tag("a",rules::opening_and_closing);
			r.add_tag("b",rules::opening_and_closing);
			TEST(!r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(!r.valid_boolean_property("a","src"));
			r.add_property("a","src",booster::regex(".*"));
			TEST(r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(!r.valid_property("b","src","javascript:alert('XSS')"));
			r.add_property("a","src",booster::regex("(http|ftp|https)://.*"));
			TEST(!r.valid_property("a","src","javascript:alert('XSS')"));
			TEST(!r.valid_property("a","src","javascript:alert('XSS') http://foo"));
			TEST(r.valid_property("a","src","http://foo"));
			TEST(r.valid_property("a","Src","http://foo"));
			r.add_boolean_property("a","disabled");
			TEST(!r.valid_property("a","disabled","disabled"));
			TEST(r.valid_boolean_property("a","disabled"));
			r.add_integer_property("a","border");
			TEST(r.valid_property("a","border","0"));
			TEST(r.valid_property("a","border","3415423452452454235234523"));
			TEST(r.valid_property("a","border","-3415423452452454235234523"));
			TEST(!r.valid_property("a","border","-"));
			TEST(!r.valid_property("a","border","1.3"));
			TEST(!r.valid_property("a","border",""));
		}
		std::cout <<"--- URI-Filter" << std::endl;
		{
			rules r;
			r.add_tag("a",rules::opening_and_closing);
			r.add_tag("img",rules::stand_alone);
			r.add_uri_property("a","href");
			r.add_uri_property("img","src","(http|https)");
			TEST(!r.valid_property("a","href","javascript:alert('XSS')"));
			TEST(!r.valid_property("a","href","javascript:alert(&quot;XSS&quot;)"));
			TEST(!r.valid_property("a","href","&#106;&#97;&#118;&#97;&#115;&#99;&#114;&#105;&#112;&#116;&#58;"
							  "&#97;&#108;&#101;&#114;&#116;&#40;&#39;&#88;&#83;&#83;&#39;&#41;"));
			TEST(!r.valid_property("a","href","&#x6A&#x61&#x76&#x61&#x73&#x63&#x72&#x69&#x70&#x74&#x3A&#x61&#x6C"
							  "&#x65&#x72&#x74&#x28&#x27&#x58&#x53&#x53&#x27&#x29"));
			TEST(!r.valid_property("a","href","jav&#x61;script:alert('XSS');"));
			
			TEST(!r.valid_property("a","href","javascript:alert(\"XSS\")"));
			TEST(!r.valid_property("a","href","javascript:test()"));
			TEST(!r.valid_property("a","href","http://thisisatest.com/@\"onmouseover=\"alert('test xss')\"/"));

			TEST(r.valid_property("a","href","test.html"));
			TEST(r.valid_property("a","href","/foo/test.html"));
			TEST(r.valid_property("a","href","https://127.0.0.1/foo/test.html"));
			TEST(r.valid_property("a","href","http://localhost/foo/test.html"));
			TEST(r.valid_property("a","href","http://foo:bar@localhost/foo/test.html"));
			TEST(r.valid_property("a","href","https://example.com/foo/test.html"));
			TEST(r.valid_property("a","href","https://example.com/foo/test.html#There"));
			TEST(r.valid_property("a","href","https://example.com/foo/test.html?x=a&amp;y=b#There"));
			TEST(!r.valid_property("a","href","https://example.com/foo/test.html?x=a&y=b#There"));
			TEST(r.valid_property("a","href","http://en.wikipedia.org/wiki/I'm"));
			TEST(r.valid_property("a","href","http://en.wikipedia.org/wiki/I&apos;m"));
			TEST(!r.valid_property("a","href","jav&#x0A;ascript:alert('XSS');"));
			TEST(r.valid_property("a","href","http://he.wikipedia.org/wiki/%D7%A9%D7%9C%D7%95%D7%9D?x=%D7#x"));
			TEST(r.valid_property("a","href","/wiki/%D7%A9%D7%9C%D7%95%D7%9D?x=%D7#x"));
			TEST(r.valid_property("a","href","http://en.wikipedia.org/wiki/C_(programming_language)"));
			TEST(r.valid_property("a","href","ftp://myftp/"));
			TEST(!r.valid_property("img","src","ftp://myftp/"));
			TEST(r.valid_property("img","src","http://myftp/"));
		}
	}

	
	
}

bool validate_simple(char const *s,cppcms::xss::rules const &r,char const *rm=0,char const *esc=0)
{
	bool res = cppcms::xss::validate(s,s+strlen(s),r);
	std::string tmp1,tmp2;
	bool res2 = cppcms::xss::validate_and_filter_if_invalid(s,s+strlen(s),r,tmp1,cppcms::xss::remove_invalid);
	bool res3 = cppcms::xss::validate_and_filter_if_invalid(s,s+strlen(s),r,tmp2,cppcms::xss::escape_invalid);
	TEST(res==res2);
	TEST(res==res3);
	if(!res) {
		if(rm) {
			if(tmp1!=rm)
				std::cerr <<"[" << s <<"]->[" << tmp1 << "]!=["<<rm<<"]" << std::endl;
			TEST(tmp1==rm);
		}
		if(esc)
			TEST(tmp2==esc);
		TEST(tmp1!=s);
		TEST(tmp2!=s);
		TEST(tmp1!=tmp2);
		TEST(tmp1.size() < strlen(s));
		TEST(cppcms::xss::validate(tmp1.c_str(),tmp1.c_str()+tmp1.size(),r));
		TEST(cppcms::xss::validate(tmp2.c_str(),tmp2.c_str()+tmp2.size(),r));
	}
	return res;
}

void test_validation()
{
	std::cout << "- Testing filter" << std::endl;
	using namespace cppcms::xss;
	{
		rules r;
		std::cout << "-- Testing entities" << std::endl;
		TEST(validate_simple("to be or not to be",r));
		TEST(validate_simple("to be &amp; not to be",r));
		TEST(!validate_simple("to be &or; not to be",r,"to be  not to be","to be &amp;or; not to be"));
		TEST(!validate_simple("&or; not to be",r));
		TEST(!validate_simple("to be &or;",r));
		TEST(!validate_simple("&amp not to be",r,"","&amp;amp not to be"));
		TEST(!validate_simple("to be &amp",r,"to be ","to be &amp;amp"));
		r.add_entity("or");
		TEST(validate_simple("to be &or; not to be",r));
		TEST(validate_simple("&or; not to be",r));
		TEST(validate_simple("to be &or;",r));

		TEST(!validate_simple("&#64;",r));
		TEST(!validate_simple("&#x3c;",r));
		TEST(!validate_simple("&#X3c;",r));
		r.numeric_entities_allowed(true);
		TEST(validate_simple("&#64;",r));
		TEST(validate_simple("&#x3c;",r));
		TEST(validate_simple("&#X3c;",r));
		TEST(!validate_simple("&#x3;",r));
		TEST(validate_simple("&#x0A;",r));
		TEST(!validate_simple("&#xFFFF;",r));
		TEST(!validate_simple("&#xFFFE;",r));
		TEST(!validate_simple("&#xDB01;",r));
		TEST(validate_simple("&#x10FFFF;",r));
		TEST(!validate_simple("&#x110000;",r));
		TEST(!validate_simple("&#10000000000000;",r));
		TEST(!validate_simple("&#-45;",r));
	}
	{
		rules r;
		std::cout << "-- Testing basic tags" << std::endl;
		TEST(!validate_simple("<hr />",r));
		r.add_tag("hr",rules::stand_alone);
		TEST(validate_simple("<hr />",r));
		TEST(validate_simple("<hr/>",r));
		TEST(!validate_simple("< hr/>",r));
		TEST(!validate_simple("<hr / >",r));
		TEST(!validate_simple("<hr/ >",r));

		TEST(validate_simple("xxx<hr/>xxx",r));
		TEST(!validate_simple("<hr>",r));
		TEST(!validate_simple("</hr>",r));
		TEST(!validate_simple("<hr></hr>",r));
		TEST(!validate_simple("<hr",r,"","&lt;hr"));
		TEST(!validate_simple("hr>",r,"hr","hr&gt;"));
		r.add_tag("a",rules::opening_and_closing);
		TEST(!validate_simple("<a/>",r));
		TEST(validate_simple("<a></a>",r));
		r.add_tag("p",rules::any_tag);
		TEST(validate_simple("<p/>",r));
		TEST(validate_simple("<p></p>",r));
		TEST(validate_simple("<p></p>",r));
	}
	{
		std::cout << "-- Testing properties" << std::endl;
		rules r;
		r.add_tag("hr",rules::stand_alone);
		r.add_tag("a",rules::opening_and_closing);
		TEST(!validate_simple("<hr test='test' />",r));
		r.add_boolean_property("hr","test");
		r.add_integer_property("hr","size");
		r.add_uri_property("a","href");
		TEST(validate_simple("<hr size='1' />",r));
		TEST(validate_simple("<hr test='test' />",r));
		TEST(validate_simple("<hr test='test' size=\"1\" />",r));
		TEST(!validate_simple("<hr test='test' size=\"1\" >",r));
		TEST(!validate_simple("<hr test='test' size=1 />",r));
		TEST(!validate_simple("<hr test='test' size='x'/>",r));
		TEST(!validate_simple("<hr test='test' size='1' size='2' />",r));
		TEST(!validate_simple("<hr size/>",r));
		TEST(!validate_simple("<hr size />",r));
		TEST(!validate_simple("<hr size ='10'  />",r));
		TEST(!validate_simple("<hr size= '10'  />",r));
		TEST(!validate_simple("<hr size='10'test='test'  />",r));
		TEST(validate_simple("<a href='http://google.com'>x</a>",r));
		TEST(!validate_simple("<a href='javascript:xss()'>x</a>",r,"x"));
		TEST(!validate_simple("<a>x</a href='http://google.com'>",r,"x"));
	}
	{
		std::cout << "-- Testing comments" << std::endl;
		rules r;
		TEST(!validate_simple("Hello <!-- test --> world",r,"Hello  world","Hello &lt;!-- test --&gt; world"));
		r.comments_allowed(true);
		TEST(validate_simple("Hello <!-- test --> world",r));
		TEST(!validate_simple("Hello < !-- test --> world",r,"Hello  world","Hello &lt; !-- test --&gt; world"));
		TEST(!validate_simple("Hello <!- - test --> world",r));
		TEST(!validate_simple("Hello <!-- ---> world",r));
		TEST(!validate_simple("Hello <!-- -- -> world",r));
		TEST(validate_simple("Hello <!-- - --> world",r));
		TEST(!validate_simple("Hello <!-- -- --> world",r));
		TEST(validate_simple("Hello <!-- <test /> --> world",r));
		TEST(validate_simple("Hello <!-- < --> world",r));
		TEST(validate_simple("Hello <!-- > --> world",r));
		TEST(validate_simple("Hello <!----> world",r));
		TEST(validate_simple("<!-- test -->",r));
		TEST(validate_simple("<!-- test -->x",r));
		TEST(validate_simple("x<!-- test -->",r));
		TEST(!validate_simple("<!-- test --",r,""));
		TEST(!validate_simple("<!-- test -",r,""));
		TEST(!validate_simple("< !-- test -->",r,""));
	}
	{
		std::cout << "-- Test nesting" << std::endl;
		{
			rules r;
			std::cout << "--- XHTML" << std::endl;
			r.add_tag("input",rules::any_tag);
			r.add_tag("ul",rules::opening_and_closing);
			r.add_tag("ol",rules::opening_and_closing);
			r.add_tag("hr",rules::stand_alone);
			TEST(validate_simple("<ul><input /><input></input><ol><hr/></ol></ul>",r));
			TEST(!validate_simple("<ul><input /><input></input><ol><hr/></ol></ol>",r,
						"<input /><input></input><ol><hr/></ol>"));
			TEST(!validate_simple("<ul><input /><input></input><ol><hr/></ul></ul>",r,
						"<ul><input /><input></input><hr/></ul>"));
			TEST(!validate_simple("<ul><input /><input></input><ul><hr/></ol></ol>",r,
						"<input /><input></input><hr/>"
						));
			TEST(!validate_simple("<ul><input /><input></input><ol><hr/></ul></ol>",r,
						"<input /><input></input><hr/>"
						));
			TEST(!validate_simple("<ul><input /><input></input><ol><hr>x</hr></ol></ul>",r,
						"<ul><input /><input></input><ol>x</ol></ul>"
						));
		}
		{
			rules r;
			r.html(rules::html_input);
			std::cout << "--- HTML" << std::endl;
			r.add_tag("p",rules::any_tag);
			r.add_tag("b",rules::opening_and_closing);
			r.add_tag("i",rules::opening_and_closing);
			r.add_tag("hr",rules::stand_alone);
			TEST(validate_simple("<p>x<p>y<p>z",r));
			TEST(validate_simple("<p>x</p>y<p>z</p>",r));
			TEST(validate_simple("<b> y<p>z</B>",r));
			TEST(validate_simple("<i><b> y<p>z<hr></B></i>",r));
			TEST(!validate_simple("<b><i> y<p>z</B></i>",r,"<b> y<p>z</B>"));
			TEST(!validate_simple("<i><b> y<p>z<hr></hr></B></i>",r,"<i><b> y<p>z</B></i>"));
		}
	}	
}

int main()
{
	try {
		test_rules();
		test_validation();
	}
	catch(std::exception const &e){
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
