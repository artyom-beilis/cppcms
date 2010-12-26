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
	}

	
	
}

bool validate_simple(char const *s,cppcms::xss::rules const &r)
{
	bool res = cppcms::xss::validate(s,s+strlen(s),r);
	std::string tmp;
	bool res2 = cppcms::xss::validate_and_filter_if_invalid(s,s+strlen(s),r,tmp);
	TEST(res==res2);
	if(!res) {
		TEST(tmp!=s);
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
		TEST(!validate_simple("to be &or; not to be",r));
		TEST(!validate_simple("&or; not to be",r));
		TEST(!validate_simple("to be &or;",r));
		TEST(!validate_simple("&amp not to be",r));
		TEST(!validate_simple("to be &amp",r));
		r.add_entity("or");
		TEST(validate_simple("to be &or; not to be",r));
		TEST(validate_simple("&or; not to be",r));
		TEST(validate_simple("to be &or;",r));
	}
	{
		rules r;
		std::cout << "-- Testing basic tags" << std::endl;
		TEST(!validate_simple("<hr />",r));
		r.add_tag("hr",rules::stand_alone);
		TEST(validate_simple("<hr />",r));
		TEST(validate_simple("<hr/>",r));
		TEST(validate_simple("xxx<hr/>xxx",r));
		TEST(!validate_simple("<hr>",r));
		TEST(!validate_simple("</hr>",r));
		TEST(!validate_simple("<hr></hr>",r));
		TEST(!validate_simple("<hr",r));
		TEST(!validate_simple("hr>",r));
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
		TEST(!validate_simple("<hr test='test' />",r));
		r.add_boolean_property("hr","test");
		r.add_integer_property("hr","size");
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