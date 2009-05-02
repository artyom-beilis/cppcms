#include "http_request.h"
#include <boost/noncopyable.hpp>

namespace cppcms { namespace http {

request::~request()
{
}

request::request(connection &conn) :
	conn_(conn)
{
	input_.reset(new input(conn));
}


} } // cppcms::http
