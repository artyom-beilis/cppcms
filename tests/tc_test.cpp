#include "tc_test_content.h"
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/json.h>
#include <cppcms/url_mapper.h>
#include "dummy_api.h"
#include "test.h"

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
		cppcms::service srv(cfg);
		test_app app(srv);

		app.set_context();

		app.test_skins();
		app.test_views();
		app.test_templates();
		app.test_if();
		app.test_url();
		app.test_foreach();

	}
	catch(std::exception const &e)
	{
		std::cerr << "Fail " << e.what() << std::endl;
		return 1;
	}
	std::cout << "Ok" << std::endl;
	return 0;
}



