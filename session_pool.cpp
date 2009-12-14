#define CPPCMS_SOURCE
#include "session_pool.h"

namespace cppcms {
struct session_pool::data {};

session_pool::session_pool(service &unused)
{
}

session_pool::~session_pool()
{
}

intrusive_ptr<session_api> session_pool::get()
{
	return 0;
}

} /// cppcms
