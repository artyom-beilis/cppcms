#include <service.h>
#include <application.h>
#include <applications_pool.h>
#include <http_request.h>
#include <http_response.h>
#include <http_context.h>
#include <json.h>
#include <iostream>

class unit_test : public cppcms::application {
public:
	unit_test(cppcms::service &s) : cppcms::application(s)
	{
	}
	virtual void main(std::string /*unused*/)
	{
		response().set_plain_text_header();
		if(!is_asynchronous())
			response().io_mode(cppcms::http::response::nogzip);
		std::map<std::string,std::string> env=request().getenv();
		std::ostream &out = response().out();
		for(std::map<std::string,std::string>::const_iterator p=env.begin();p!=env.end();++p) {
			out << p->first <<':'<<p->second << '\n';
		}
		out << '\n';
		typedef cppcms::http::request::form_type form_type;
		form_type const &form=request().post();
		for(form_type::const_iterator p=form.begin();p!=form.end();++p) {
			out << p->first <<'='<<p->second << '\n';
		}
		if(is_asynchronous())
			release_context()->async_complete_response();
	}
};





int main(int argc,char **argv)
{
	try {
		cppcms::service srv(argc,argv);
		cppcms::intrusive_ptr<cppcms::application> app;
		if(!srv.settings().get("test.async",false)) {
			srv.applications_pool().mount( cppcms::applications_factory<unit_test>());
		}
		else {
			app=new unit_test(srv);
			srv.applications_pool().mount(app);
		}
		srv.run();
	}
	catch(std::exception const &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
