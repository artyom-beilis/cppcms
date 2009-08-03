#define CPPCMS_SOURCE

#include "cgi_api.h"
#include "service.h"
#include "http_context.h"
#include "http_request.h"
#include "http_response.h"

namespace cppcms {
namespace http {

	struct context::data {
		http::request request;
		http::response response;
		data(context &cntx) :
			request(cntx.connection()),
			response(cntx)
		{
		}
	};

context::context(impl::cgi::connection *conn) :
	conn_(conn)
{
	d.reset(new data(*this));	
}

context::~context()
{
}


impl::cgi::connection &context::connection()
{
	return *conn_;
}

http::request &context::request()
{
	return d->request;
}

http::response &context::response()
{
	return d->response;
}

cppcms_config const &context::settings()
{
	return conn_->service().settings();
}


} // http
} // cppcms
