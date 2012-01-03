///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include "tc_test_content.h"
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/json.h>
#include <cppcms/cache_interface.h>
#include <cppcms/url_mapper.h>
#include "dummy_api.h"
#include "test.h"

#include <iomanip>
#include <sstream>

int calls_done = 0;
void call_recorder()
{
	calls_done ++;
}

std::string remove_space(std::string const &input)
{
	std::string r;
	for(size_t i=0;i<input.size();i++) {
		if(input[i]!=' ')
			r += input[i];
	}
	return r;
}

std::string conv(std::string const &l,size_t i)
{
	if(i >= l.size()) {
		return "---";
	}
	if(l[i] == '\n')
		return "\\n";
	if(l[i] < 32) {
		std::ostringstream ss;
		ss <<"\\x" << std::hex << (int)(l[i]);
		return ss.str();
	}
	else {
		return l.substr(i,1);
	}
}

void compare_strings(std::string const &l,std::string const &r)
{
	if(l==r)
		return;
	size_t m = l.size();
	if(r.size() > m)  m = r.size();
	int line = 1;
	for(size_t i=0;i<m;i++) {
		std::string lstr = conv(l,i);
		std::string rstr = conv(r,i);
		if(lstr=="\\n")
			line++;
		std::cerr << std::setw(4) << line << " [" << lstr << '|' << rstr << "]   ";
		if(lstr!=rstr)
			std::cerr << "<----------" << std::endl;
		else
			std::cerr << std::endl;
	}
	std::cerr << "[" << l << "]!=[" << r << "]" << std::endl;
	throw std::runtime_error("Failed test");

}

class test_app : public cppcms::application {
public:
	test_app(cppcms::service &srv) : 
		cppcms::application(srv),
		srv_(srv)
	{
		mapper().assign("foo","/foo");
		mapper().assign("/");
		mapper().assign("/{1}");
		mapper().assign("/{1}/{2}");
	}
	~test_app()
	{
		release_context();
	}
	void set_context()
	{
		std::map<std::string,std::string> env;
		env["HTTP_HOST"]="www.example.com";
		env["SCRIPT_NAME"]="/foo";
		env["PATH_INFO"]="/bar";
		env["REQUEST_METHOD"]="GET";
		booster::shared_ptr<dummy_api> api(new dummy_api(srv_,env,output_));
		booster::shared_ptr<cppcms::http::context> cnt(new cppcms::http::context(api));
		assign_context(cnt);
		response().out();
		output_.clear();
	}

	std::string str()
	{
		response().out() << std::flush;
		std::string result = output_;
		output_.clear();
		return result;
	}

	void test_skins()
	{
		std::cout << "- Testing different skins" << std::endl;
		data::master m;
		render("tc_skin_a","master",m);
		TEST(str()=="a");
		render("tc_skin_b","master",m);
		TEST(str()=="b");
		render("test_default_master",m);
		TEST(str()=="c");
	}
	void test_views()
	{
		std::cout << "- Testing different views" << std::endl;
		data::master m;
		render("view_x",m);
		TEST(str()=="view x");
		render("view_y",m);
		TEST(str()=="view y");
	}
	void test_templates()
	{
		std::cout << "- Testing different template calls" << std::endl;
		data::master m;
		m.integer = 1;
		m.text = "str";
		m.integers.push_back(21);
		m.integers.push_back(22);
		render("master_tmpl",m);
		TEST(str()=="\nA\nx=10\nx=1\nx=10 y=test\nx=1 y=str\n");
	}
	void test_foreach()
	{
		std::cout << "- Testing foreach" << std::endl;
		data::master m;
		m.integers.push_back(21);
		m.integers.push_back(22);
		render("foreach_tmpl",m);
		TEST(str() ==	"\n"
				"{21,22}\n"
				"{22,21}\n"
				"{21 0,22 1}\n"
				"{22 0,21 1}\n"
				"{21 1,22 2}\n"
				"{22 1,21 2}\n"
				"\n"
				"{ 21 22}\n"
				"{ 22 21}\n"
				"{ 21 0 22 1}\n"
				"{ 22 0 21 1}\n"
				"{ 21 1 22 2}\n"
				"{ 22 1 21 2}\n"
				"\n"
				"{21,22}\n"
				"{22,21}\n"
				"{21 0,22 1}\n"
				"{22 0,21 1}\n"
				"{21 1,22 2}\n"
				"{22 1,21 2}\n"
				"\n"
				"{ 21 22}\n"
				"{ 22 21}\n"
				"{ 21 0 22 1}\n"
				"{ 22 0 21 1}\n"
				"{ 21 1 22 2}\n"
				"{ 22 1 21 2}\n");
		m.integers.clear();
		render("foreach_tmpl",m);
		TEST(str()=="\n"
			"-\n"
			"-\n"
			"-\n"
			"-\n"
			"-\n"
			"-\n"
			"\n"
			"-\n"
			"-\n"
			"-\n"
			"-\n"
			"-\n"
			"-\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
			"\n"
		);
	}
	void test_if()
	{
		std::cout << "- Testing conditions" << std::endl;
		data::master m;
		m.integer = 1;
		m.text = "x";
		render("master_if",m);
		TEST(str()=="\ninteger\n!!integer\ntext not empty\n!text empty\ntrue\n\ntrue\n");
		m.integer = 0;
		m.text = "";
		render("master_if",m);
		TEST(str()=="\n!integer\n!integer\ntext empty\n!!text not empty\ntrue\n\ntrue\n");
	}
	void test_url()
	{
		std::cout << "- Testing url" << std::endl;
		data::master m;
		m.integer = 1;
		m.text = "/";
		render("master_url",m);
		TEST(str()=="\n"
			"/\n"
			"/1\n"
			"/1/%2f\n"
			"/1\n"
			"/1/%2f\n"
			"/1//\n"
			"/foo\n");
	}
	void test_format()
	{
		std::cout << "- Testing formatting" << std::endl;
		data::master m;
		m.integer = 1;
		m.text = "<abAB>";
		render("master_filter",m);
		compare_strings(str(),"\n" 
			"1\n" 		// <%= integer %>
			"  1\n" 	// <% gt "{1,w=3}" using integer %>
			"  1 &lt;abAB&gt;\n"
					// <% gt "{1,w=3} {2}" using integer , text %>
			"<abAB>\n"	// <% gt "{1}" using text | raw %>
			"<baAB>\n"	// <% gt "{1}" using text | test_filter %>
			"&lt;baAB&gt;\n"// <% gt "{1}" using text | test_filter | escape %>
			"&lt;baAB&gt; 1\n"
					// <% gt "{1} {2}" using text | test_filter | escape , integer %>
			"&lt;abAB&gt;\n"// <%=text %>
			"<abAB>\n"	// <%=text | raw %>
			"<baAB>\n"	// <%=text | test_filter %>
			"&lt;baAB&gt;\n"// <%=text | test_filter | escape %>
			"%3cabAB%3e\n"	// <%=text | urlencode %>
			"PGFiQUI-\n"	// <%=text | base64_urlencode %>
			);
	}

	void test_forms()
	{
		std::cout << "- Testing forms" << std::endl;
		data::form_test m;
		render("form_test",m);
		compare_strings(remove_space(str()),remove_space("\n" 
				"<p>msg &nbsp; <span class=\"cppcms_form_input\">"
				" <input type=\"text\" name=\"_1\"  > </span>"
				" <span class=\"cppcms_form_help\">help</span></p>\n"
				"<p><span class=\"cppcms_form_input\">"
				" <input type=\"text\" name=\"_2\" ></span></p>\n\n"
				
						// <% form as_space f %>
				"<input type=\"text\" name=\"_1\" >\n"
					// <% form input t1 %>
				"<input type=\"text\" name=\"_2\" >\n"
					// <% form input t2 %>
				"<input type=\"text\" name=\"_2\" prop >\n"
					// <% form begin t2 %> prop <% form end t2 %>
				"<input type=\"text\" name=\"_2\" id='x' >\n"
					// <% form block t2 %> id='x' <% end form %>
				)
			);

	}

	void test_cache()
	{
		std::cout << "- Testing cache" << std::endl;
		data::form_test m;
		m.integer = 1;
		m.call = call_recorder;
		render("cache_test",m);
		TEST(calls_done == 1);
		TEST(str()=="1");
		m.integer = 2;
		render("cache_test",m);
		TEST(calls_done == 1);
		TEST(str()=="1");
		cache().rise("tr");
		render("cache_test",m);
		TEST(calls_done == 2);
		TEST(str()=="2");
		cache().rise("tr");
		m.integer=3;
		render("cache_test_nr",m);
		TEST(str()=="3");
		cache().rise("tr");
		m.integer=4;
		{
			cache().reset();
			cppcms::triggers_recorder tr(cache());
			render("cache_test_nr",m);
			TEST(str()=="3");
			TEST(tr.detach().count("key") == 1);
		}
		{
			cache().rise("key");
			cache().reset();
			cppcms::triggers_recorder tr(cache());
			render("cache_test_nr",m);
			TEST(str()=="4");
			TEST(tr.detach().count("key") == 1);
		}
		{
			cache().clear();
			m.integer=1;
			cppcms::triggers_recorder tr(cache());
			render("cache_test_nt",m);
			TEST(str()=="1");
			TEST(tr.detach().count("key") == 0);
		}

	}
	
	void test_using_render()
	{
		std::cout << "- Testing using and render" << std::endl;
		data::master m;
		m.integer = 10;
		m.h.x = 1;
		m.h.y = 2;
		m.text = "helper_helper";
		render("test_using", m );
		compare_strings(str(),"\n"
			"r 1; 2\n"	// <% render "tc_skin", "helper_helper" with h %>
			"ri 10\n"	// <% render "tc_skin", "helper_master" %>
			"r 1; 2\n"	// <% render "helper_helper" with h %>
			"r 1; 2\n"	// <% render text with h %>
			"ri 10\n"	// <% render "helper_master" %>
			"\n"		// <% using helper_helper with h as hlp %>
			"1\n"		// <% include show_x() from hlp %>
			"2\n"		// <% include show_y() from hlp %>
			"1; 2\n"	// <% include show_x(h.y) from hlp %>
			"2; 1\n"	// <% include show_y(h.x) from hlp %>
			"\n"		// <% end using %>
			"\n"		// <% using helper_master as hlp %>
			"i 10\n"	// <% include show_integer() from hlp %>
			"\n"		// <% end using %>
			"1\n"		// <% include show_x() using helper_helper with h %>
			"2\n"		// <% include show_y() using helper_helper with h %>
			"1; 2\n"	// <% include show_x(h.y) using helper_helper with h %>
			"2; 1\n"	// <% include show_y(h.x) using helper_helper with h %>
			"i 10\n"	// <% include show_integer() using helper_master %>
		);
	}


private:
	std::string output_;
	cppcms::service &srv_;
};


int main()
{
	try {
		cppcms::json::value cfg;
		cfg["views"]["paths"][0]="./";
		cfg["views"]["skins"][0]="tc_skin_a";
		cfg["views"]["skins"][1]="tc_skin_b";
		cfg["views"]["skins"][2]="tc_skin";
		cfg["views"]["default_skin"]="tc_skin";
		cfg["cache"]["backend"]="thread_shared";
		cfg["cache"]["limit"]=100;
		cppcms::service srv(cfg);
		test_app app(srv);

		app.set_context();

		app.test_skins();
		app.test_views();
		app.test_templates();
		app.test_if();
		app.test_url();
		app.test_foreach();
		app.test_format();
		app.test_forms();
		app.test_cache();
		app.test_using_render();
	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}



