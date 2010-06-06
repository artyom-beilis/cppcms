#include "mb.h"
#include "master_content.h"

#include "forums.h"
#include "thread.h"
#include <cppcms/json.h>

namespace apps {

mb::mb(cppcms::service &w) :
	cppcms::application(w),
	forums(*this),
	thread(*this)
{	
	add(forums);
	add(thread);
	sql.driver("sqlite3");
	sql.param("sqlite3_dbdir","./");
	sql.param("dbname","mb.db");
	sql.connect();
}

void mb::ini(content::master &c)
{
	c.main_page=request().script_name()+"/";
	c.media=settings().get<std::string>("mb.media");
}


} // namespace apps
