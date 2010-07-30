#include <cppcms/application.h>
#include <cppcms/service.h>
#include <cppcms/applications_pool.h>
#include <cppcms/rpc_json.h>


class json_service : public cppcms::rpc::json_rpc_server {
public:
    json_service(cppcms::service &srv) : cppcms::rpc::json_rpc_server(srv)
    {
    	bind("sum",cppcms::rpc::json_method(&json_service::sum,this),method_role);
    	bind("div",cppcms::rpc::json_method(&json_service::div,this),method_role);
    	bind("notify",cppcms::rpc::json_method(&json_service::div,this),notification_role);
    	bind("both",cppcms::rpc::json_method(&json_service::both,this));
    }
    void sum(int x,int y)
    {
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
        std::cout << "We got notification " << msg << std::endl;
    }
    void both(std::string msg)
    {
    	if(notification())
            std::cout << "We got notification " << msg << std::endl;
    	else
    		return_result("call:"+msg);
    }
};

int main(int argc,char **argv)
{
    try {
    	cppcms::service srv(argc,argv);
    	srv.applications_pool().mount( cppcms::applications_factory<json_service>());
    	srv.run();
    }
    catch(std::exception const &e) {
    	std::cerr << e.what() << std::endl;
    	return 1;
    }
    return 0;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
