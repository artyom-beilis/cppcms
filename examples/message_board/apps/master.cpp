#include "master.h"

#include <cppcms/json.h>
#include <cppcms/session_interface.h>
#include <cppcms/url_mapper.h>
#include <iostream>

namespace apps {

master::master(cppcms::service &srv) : cppcms::application(srv)
{	
}

void master::init()
{
	sql.open("sqlite3:db=mb.db");
	if(!session().is_set("view") || session()["view"]=="tree") {
		parent()->mapper().set_value("method","tree");
	}
	else {
		parent()->mapper().set_value("method","flat");
	}
}
void master::clear()
{
	sql.close();
}

void master::prepare(data::master &c)
{
	c.media=settings().get<std::string>("mb.media");
}


} // namespace apps
