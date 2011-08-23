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
#include <cppcms/json.h>
#include <booster/locale/encoding.h>
#include "test.h"
#include "c_string.h"
#include <iostream>
#include <string.h>
#include <fstream>
#include <cstdlib>

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
		TEST(validate_simple("<a href='http://google.com/foo?x=y&amp;bar=x'>x</a>",r));
		TEST(validate_simple("<a href=\"http://cppcms.sourceforge.net/banner-small.png\">x</a>",r));
		TEST(validate_simple("<a href=\"#Are+you+crazy+or+masochist?\">x</a>",r));
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
		TEST(!validate_simple("Hello <!-- <test /> --> world",r,"Hello  world"));
		TEST(!validate_simple("Hello <!-- < --> world",r,"Hello  world"));
		TEST(!validate_simple("Hello <!-- > --> world",r,"Hello  world"));
		TEST(validate_simple("Hello <!----> world",r));
		TEST(validate_simple("<!-- test -->",r));
		TEST(validate_simple("<!-- test -->x",r));
		TEST(validate_simple("x<!-- test -->",r));
		TEST(!validate_simple("<!-- test --",r,""));
		TEST(!validate_simple("<!-- test -",r,""));
		TEST(!validate_simple("< !-- test -->",r,""));
		TEST(!validate_simple("a<!--[if gte IE 4]><SCRIPT>alert('XSS');</SCRIPT><![endif]-->b",r,"ab"));
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

void printhex(std::string const &s)
{
	for(unsigned i=0;i<s.size();i++) {
		unsigned char c=s[i];
		std::cout << std::hex << "\\x" << unsigned(c);
	}
	std::cout << std::endl;
}

bool valenc(char const *s,cppcms::xss::rules const &r,char const *rm=0,char const *esc=0,char const *rmalt=0)
{
	bool res = cppcms::xss::validate(s,s+strlen(s),r);
	std::string tmp1,tmp2;
	bool res2 = cppcms::xss::validate_and_filter_if_invalid(s,s+strlen(s),r,tmp1,cppcms::xss::remove_invalid);
	bool res3 = cppcms::xss::validate_and_filter_if_invalid(s,s+strlen(s),r,tmp2,cppcms::xss::escape_invalid,'?');
	TEST(res == res2);
	TEST(res == res3);
	if(!res) {
		if(rm && rmalt) {
			TEST(tmp1==rm || tmp1==rmalt);
		}
		else if(rm) {
			TEST(tmp1==rm);
		}
		if(esc)
			TEST(tmp2==esc);
		TEST(tmp1!=s);
		TEST(tmp2!=s);

		if(rm && esc && strcmp(rm,esc)!=0) {
			// encoding sometimes always remove
			TEST(tmp1!=tmp2);
			TEST(tmp1.size() < strlen(s));
		}
		TEST(cppcms::xss::validate(tmp1.c_str(),tmp1.c_str()+tmp1.size(),r));
		TEST(cppcms::xss::validate(tmp2.c_str(),tmp2.c_str()+tmp2.size(),r));
	}
	return res;

}


void test_encoding()
{
	std::cout << "- Testing encoding validation" << std::endl;
	using namespace cppcms::xss;
	{
		rules r;
		r.encoding("UTF-8");
		std::cout << "-- Testing UTF-8" << std::endl;
		TEST(valenc("שלום עולם",r));
		TEST(!valenc("שלום \xFF" "עולם",r,"שלום עולם","שלום ?עולם"));
		TEST(!valenc("שלום \4עולם",r,"שלום עולם","שלום ?עולם"));
		TEST(!valenc("\xFF" "עולם",r,"עולם","?עולם"));
		TEST(!valenc("\4עולם",r,"עולם","?עולם"));
		TEST(!valenc("שלום \xFF",r,"שלום ","שלום ?"));
		TEST(!valenc("שלום \4",r,"שלום ","שלום ?"));
	}
	{
		rules r;
		r.encoding("ISO-8859-8");
		std::cout << "-- Testing ISO-8859-8" << std::endl;
		TEST(valenc("\xf9\xec\xe5\xed \xf2\xe5\xec\xed",r));
		TEST(!valenc("\xf9\xec\xe5\xed \xc8\xf2\xe5\xec\xed",r,"\xf9\xec\xe5\xed \xf2\xe5\xec\xed","\xf9\xec\xe5\xed ?\xf2\xe5\xec\xed"));
		TEST(!valenc("\xf9\xec\xe5\xed \4\xf2\xe5\xec\xed",r,"\xf9\xec\xe5\xed \xf2\xe5\xec\xed","\xf9\xec\xe5\xed ?\xf2\xe5\xec\xed"));
		TEST(!valenc("\xc8\xf9\xec\xe5\xed \xf2\xe5\xec\xed",r,"\xf9\xec\xe5\xed \xf2\xe5\xec\xed","?\xf9\xec\xe5\xed \xf2\xe5\xec\xed"));
		TEST(!valenc("\4\xf9\xec\xe5\xed \xf2\xe5\xec\xed",r,"\xf9\xec\xe5\xed \xf2\xe5\xec\xed","?\xf9\xec\xe5\xed \xf2\xe5\xec\xed"));
		TEST(!valenc("\xf9\xec\xe5\xed \xc8",r,"\xf9\xec\xe5\xed ","\xf9\xec\xe5\xed ?"));
		TEST(!valenc("\xf9\xec\xe5\xed \4",r,"\xf9\xec\xe5\xed ","\xf9\xec\xe5\xed ?"));
	}
	{
		try {
			rules r;
			r.encoding("windows-932");
			std::cout << "-- Testing windows-932" << std::endl;
			TEST(valenc("aX\xCB\x83\x71\x82\xd0",r));
			// Only remove
			// single invalid code point in the middle 0xa0
			TEST(!valenc("aX\xCB\x83\x71\xa0\x82\xd0",r,"aX\xCB\x83\x71\x82\xd0","aX\xCB\x83\x71\x82\xd0"));
			// single invalid code point in the end 0xa0
			TEST(!valenc("aX\xCB\x83\x71\xa0",r,"aX\xCB\x83\x71","aX\xCB\x83\x71"));
			// single invalid code point in the begin 0xa0
			TEST(!valenc("\xa0\x82\xd0",r,"\x82\xd0","\x82\xd0"));
			// incompete code point in the end 0x82xd0
			TEST(!valenc("aX\xCB\x83\x71\x82\xd0\x82",r,"aX\xCB\x83\x71\x82\xd0","aX\xCB\x83\x71\x82\xd0"));
			// single invalid code point in the middle 4
			TEST(!valenc("aX\xCB\x83\x71\4\x82\xd0",r,"aX\xCB\x83\x71\x82\xd0","aX\xCB\x83\x71\x82\xd0"));
			// single invalid code point in the end 4
			TEST(!valenc("aX\xCB\x83\x71\4",r,"aX\xCB\x83\x71","aX\xCB\x83\x71"));
			// single invalid code point in the begin 4
			TEST(!valenc("\4\x82\xd0",r,"\x82\xd0","\x82\xd0"));
			// single invalid code point in the middle 83 f0
			TEST(!valenc(	"aX\xCB\x83\x71\x83\xf0\x82\xd0",r,
					"aX\xCB\x83\x71\x82\xd0",
					0, // both are legal ways to interpret as xf0x82 + d0 and x82xd0 are valid code points
					"aX\xCB\x83\x71\xf0\x82\xd0"));
			// single invalid code point in the end 83 f0
			TEST(!valenc("aX\xCB\x83\x71\x83\xf0",r,"aX\xCB\x83\x71","aX\xCB\x83\x71"));
			// single invalid code point in the begin 83 f0
			TEST(!valenc(	"\x83\xf0\x82\xd0",r,
					"\x82\xd0", 
					0, // both are legal ways to interpret as xf0x82 + d0 and x82xd0 are valid code points
					"\xf0\x82\xd0"));
		}
		catch(booster::locale::conv::invalid_charset_error const &e) {
			std::cout << "--- windows-932 charset is not supported" << std::endl;
		}
	}
}

void test_json()
{
	std::cout << "- Testing json format" << std::endl;
	using namespace cppcms::xss;
	
	{
		std::cout << "-- Testing basic properties" << std::endl;
		{
			cppcms::json::value v;
			v["xhtml"]=true;
			v["comments"]=true;
			v["numeric_entities"]=true;
			rules r(v);
			TEST(r.html() == rules::xhtml_input);
			TEST(r.comments_allowed());
			TEST(r.numeric_entities_allowed());
		}
		{
			cppcms::json::value v;
			v["xhtml"]=false;
			v["comments"]=false;
			v["numeric_entities"]=false;
			rules r(v);
			TEST(r.html() == rules::html_input);
			TEST(!r.comments_allowed());
			TEST(!r.numeric_entities_allowed());
		}
		{
			cppcms::json::value v=cppcms::json::object();
			rules r(v);
			TEST(r.html() == rules::xhtml_input);
			TEST(!r.comments_allowed());
			TEST(!r.numeric_entities_allowed());
		}
	}
	{
		std::cout << "-- Testing entities" << std::endl;
		cppcms::json::value v;
		v["entities"][0]="copy";
		v["entities"][1]="nbsp";
		rules r(v);
		TEST(r.valid_entity(details::c_string("copy")));
		TEST(r.valid_entity(details::c_string("nbsp")));
		TEST(!r.valid_entity(details::c_string("foo")));
	}
	{
		std::cout <<"-- Testing tags" << std::endl;
		{
			cppcms::json::value v;
			v["tags"]["opening_and_closing"][0]="b";
			v["tags"]["opening_and_closing"][1]="i";
			v["tags"]["stand_alone"][0]="br";
			v["tags"]["stand_alone"][1]="hr";
			v["tags"]["any_tag"][0]="input";
			rules r(v);
			TEST(r.valid_tag("b")==rules::opening_and_closing);
			TEST(r.valid_tag("i")==rules::opening_and_closing);
			TEST(r.valid_tag("hr")==rules::stand_alone);
			TEST(r.valid_tag("br")==rules::stand_alone);
			TEST(r.valid_tag("input")==rules::any_tag);
		}
	}
	{
		std::cout <<"-- Testing properties" << std::endl;
		{
			std::cout <<"--- Testing properties setting" << std::endl;
			cppcms::json::value v;
			
			v["tags"]["opening_and_closing"][0]="b";
			v["tags"]["opening_and_closing"][1]="i";
			v["tags"]["opening_and_closing"][2]="a";
			
			v["attributes"][0]["tags"][0]="b";
			v["attributes"][0]["tags"][1]="i";
			
			v["attributes"][0]["attributes"][0]="class";
			v["attributes"][0]["attributes"][1]="id";
			
			v["attributes"][0]["pairs"][0]["tag"]="a";
			v["attributes"][0]["pairs"][0]["attr"]="name";
			
			v["attributes"][0]["pairs"][1]["tag"]="b";
			v["attributes"][0]["pairs"][1]["attr"]="foo";

			v["attributes"][0]["type"]="regex";
			v["attributes"][0]["expression"]="[a-z]+";

			rules r(v);
			TEST(r.valid_property("b","class","x"));
			TEST(r.valid_property("b","id","x"));

			TEST(r.valid_property("i","class","x"));
			TEST(r.valid_property("i","id","x"));


			TEST(!r.valid_property("a","class","x"));
			TEST(r.valid_property("a","name","x"));
			TEST(r.valid_property("b","foo","x"));
		}
		{
			std::cout <<"--- Testing properties options" << std::endl;
			cppcms::json::value basic;
			basic["xhtml"]=false;
			basic["tags"]["opening_and_closing"][0]="foo";
			basic["attributes"][0]["tags"][0]="foo";
			basic["attributes"][0]["attributes"][0]="bar";
			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="boolean";
				rules r(v);
				TEST(r.valid_boolean_property("foo","bar"));
			}
			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="integer";
				rules r(v);
				TEST(r.valid_property("foo","bar","10"));
				TEST(!r.valid_property("foo","bar","xxx"));
			}
			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="regex";
				v["attributes"][0]["expression"]="[0-9a-fA-F]+";
				rules r(v);
				TEST(r.valid_property("foo","bar","10f"));
				TEST(!r.valid_property("foo","bar","x"));
			}

			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="uri";
				rules r(v);
				TEST(r.valid_property("foo","bar","/foo"));
				TEST(r.valid_property("foo","bar","http://www.google.com/foo"));
				TEST(!r.valid_property("foo","bar","javascript://www.google.com/foo"));
			}
			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="uri";
				v["attributes"][0]["scheme"]="(http|https)";
				rules r(v);
				TEST(r.valid_property("foo","bar","http://www.google.com/foo"));
				TEST(r.valid_property("foo","bar","/foo"));
				TEST(!r.valid_property("foo","bar","ftp://www.google.com/foo"));
			}
			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="absolute_uri";
				rules r(v);
				TEST(r.valid_property("foo","bar","http://www.google.com/foo"));
				TEST(!r.valid_property("foo","bar","/foo"));
				TEST(!r.valid_property("foo","bar","javascript://www.google.com/foo"));
			}
			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="absolute_uri";
				v["attributes"][0]["scheme"]="(http|https)";
				rules r(v);
				TEST(r.valid_property("foo","bar","http://www.google.com/foo"));
				TEST(!r.valid_property("foo","bar","/foo"));
				TEST(!r.valid_property("foo","bar","javascript://www.google.com/foo"));
				TEST(!r.valid_property("foo","bar","ftp://www.google.com/foo"));
			}
			{
				cppcms::json::value v=basic;
				v["attributes"][0]["type"]="relative_uri";
				rules r(v);
				TEST(r.valid_property("foo","bar","/foo"));
				TEST(r.valid_property("foo","bar","foo.txt"));
				TEST(!r.valid_property("foo","bar","http://www.google.com/foo"));
				TEST(!r.valid_property("foo","bar","javascript://www.google.com/foo"));

			}

		}
		std::cout << "-- From file" << std::endl;
		{
			cppcms::json::value v;
			v["xhtml"]=false;
			v["comments"]=true;
			v["numeric_entities"]=true;
			std::ofstream f("test.txt");
			f << v;
			f.close();
			rules r(std::string("test.txt"));
			TEST(r.html() == rules::html_input);
			TEST(r.comments_allowed());
			TEST(r.numeric_entities_allowed());
			std::remove("test.txt");
		}
	}

}

int main()
{
	try {
		test_rules();
		test_validation();
		test_encoding();
		test_json();
	}
	catch(std::exception const &e){
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}
