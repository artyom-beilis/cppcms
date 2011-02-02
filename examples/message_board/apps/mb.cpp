#include <apps/mb.h>
#include <apps/forums.h>
#include <apps/thread.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/json.h>



namespace apps {


mb::mb(cppcms::service &s) : cppcms::application(s)
{

	attach(	new forums(s),
		"forums",
		"/{1}",
		"(/(\\d+)?)?",2);

	attach( new flat_thread(s),
		"flat_thread",
		"/flat/{1}",
		"/flat/(\\d+)",1);


	attach(	new tree_thread(s),
		"tree_thread",
		"/tree/{1}",
		"/tree/(\\d+)",1);

	attach(	new reply(s),
		"comment",
		"/comment/{1}",
		"/comment/(\\d+)",1);
	
	// Generic mapping
	mapper().root(settings().get<std::string>("mb.root"));
	mapper().assign("user_thread","/{method}/{1}");
}

} // apps
