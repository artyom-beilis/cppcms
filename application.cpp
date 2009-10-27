#include "application.h"

namespace cppcms {
application::application(worker_thread &w) :
	worker(w),
	url(w.url),
	app(worker.app),
	cgi(worker.cgi),
	env(worker.env),
	cgi_conn(worker.cgi_conn),
	cache(worker.cache),
	session(worker.session),
	cout(worker.cout),
	on_start(worker.on_start),
	on_end(worker.on_end)
{
}

application::~application()
{
}

void application::on_404()
{
	add_header("Content-Type: text/html");
	set_header(new cgicc::HTTPStatusHeader(404,"Not found"));
	cout<<	"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
		"         \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
		"<html>\n"
		"  <head>\n"
		"    <title>404 - Not Found</title>\n"
		"  </head>\n"
		"  <body>\n"
		"    <h1>404 - Not Found</h1>\n"
		"  </body>\n"
		"</html>\n";
}

void application::main()
{
	on_start();
	if(url.parse()<0) {
		on_404();
	}
	on_end();
}

}
