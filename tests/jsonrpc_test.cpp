///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#include <cppcms/rpc_json.h>
#include "test.h"
#include <cppcms/applications_pool.h>

#include "client.h"


class json_service : public cppcms::rpc::json_rpc_server {
public:
	json_service(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv)
	{
		bind("sum",cppcms::rpc::json_method(&json_service::sum,this),method_role);
		bind("div",cppcms::rpc::json_method(&json_service::div,this),method_role);
		bind("notify",cppcms::rpc::json_method(&json_service::div,this),notification_role);
		bind("both",cppcms::rpc::json_method(&json_service::both,this));

		std::cout << "Checking bindings" << std::endl;
		check_method(cppcms::rpc::json_method(&json_service::compiles1c,this),"[10]");
		check_method(cppcms::rpc::json_method(&json_service::compiles2c,this),"[\"str\"]");
		check_method(cppcms::rpc::json_method(&json_service::compiles3c,this),"[[1,2]]");
		check_method(cppcms::rpc::json_method(&json_service::compiles4c,this),"[{}]");
		check_method(cppcms::rpc::json_method(&json_service::compiles5c,this),"[{\"member\":1}]");
		check_method(cppcms::rpc::json_method(&json_service::compiles6c,this),"[[1]]");
		
		check_method(cppcms::rpc::json_method(&json_service::compiles1cr,this),"[10]");
		check_method(cppcms::rpc::json_method(&json_service::compiles2cr,this),"[\"str\"]");
		check_method(cppcms::rpc::json_method(&json_service::compiles3cr,this),"[[1,2]]");
		check_method(cppcms::rpc::json_method(&json_service::compiles4cr,this),"[{}]");
		check_method(cppcms::rpc::json_method(&json_service::compiles5cr,this),"[{\"member\":1}]");
		check_method(cppcms::rpc::json_method(&json_service::compiles6cr,this),"[[1]]");

		check_method(cppcms::rpc::json_method(&json_service::compiles1v,this),"[10]");
		check_method(cppcms::rpc::json_method(&json_service::compiles2v,this),"[\"str\"]");
		check_method(cppcms::rpc::json_method(&json_service::compiles3v,this),"[[1,2]]");
		check_method(cppcms::rpc::json_method(&json_service::compiles4v,this),"[{}]");
		check_method(cppcms::rpc::json_method(&json_service::compiles5v,this),"[{\"member\":1}]");
		check_method(cppcms::rpc::json_method(&json_service::compiles6v,this),"[[1]]");

		std::cout << "Ok" << std::endl;

		smd_raw("{}");
	}

	void check_method(method_type const &m,std::string s)
	{
		std::istringstream ss(s);
		cppcms::json::value v;
		ss >> v;
		m(v.array());
	}

	void sum(int x,int y)
	{
        TEST(y!=10);
		return_result(x+y);
	}
	void div(int x,int y)
	{
		if(y==0)
			return_error("Division by zero");
		else
			return_result(x/y);
	}
	void notify(std::string msg)
	{
		TEST(msg=="notify");
	}
	void both(std::string msg)
	{
		if(notification())
			TEST(msg=="notification");
		else
			return_result("call:"+msg);
	}

	void compiles1cr(int const &f) { TEST(f==10); }
	void compiles2cr(std::string const &f) {  TEST(f=="str"); }
	void compiles3cr(std::vector<int> const &f) {  TEST(f.size()==2 && f[0]==1 && f[1]==2); }
	void compiles4cr(cppcms::json::value const &f) {  TEST(f.type()==cppcms::json::is_object && f.object().empty()); }
	void compiles5cr(cppcms::json::object const &f) { TEST(f.find("member")!=f.end() && f.find("member")->second.number()==1); }
	void compiles6cr(cppcms::json::array const &f) { TEST(f[0].number()==1); }
	
	void compiles1c(int const f) { TEST(f==10); }
	void compiles2c(std::string const f) {  TEST(f=="str"); }
	void compiles3c(std::vector<int> const f) {  TEST(f.size()==2 && f[0]==1 && f[1]==2); }
	void compiles4c(cppcms::json::value const f) {  TEST(f.type()==cppcms::json::is_object && f.object().empty()); }
	void compiles5c(cppcms::json::object const f) { TEST(f.find("member")!=f.end() && f.find("member")->second.number()==1); }
	void compiles6c(cppcms::json::array const f) { TEST(f[0].number()==1); }
	
	void compiles1v(int f) { TEST(f==10); }
	void compiles2v(std::string f) {  TEST(f=="str"); }
	void compiles3v(std::vector<int> f) {  TEST(f.size()==2 && f[0]==1 && f[1]==2); }
	void compiles4v(cppcms::json::value f) {  TEST(f.type()==cppcms::json::is_object && f.object().empty()); }
	void compiles5v(cppcms::json::object f) { TEST(f["member"].number()==1); }
	void compiles6v(cppcms::json::array f) { TEST(f[0].number()==1); }

};

int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		srv.applications_pool().mount( cppcms::applications_factory<json_service>());
		if(srv.settings().type("test.exec")!=cppcms::json::is_undefined)
			srv.after_fork(submitter(srv));
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return run_ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
