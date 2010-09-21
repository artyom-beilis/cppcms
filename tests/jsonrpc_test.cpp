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
		smd_raw("{}");
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
