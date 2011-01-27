#include "mb.h"
#include "forums.h"
#include "thread.h"
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/json.h>



namespace apps {


mb::mb(cppcms::service &s) : 
	cppcms::application(s)
{
	mapper().root(settings().get<std::string>("mb.root"));

	// Forums
	forums *forums_ptr = new forums(s);
	attach(forums_ptr);
	dispatcher().assign("(/(\\d+)?)?",&forums::prepare,forums_ptr,2);
	mapper().assign("forums","/");
	mapper().assign("forums","/{1}");


	// Flat thread
	flat_thread *flat_ptr = new flat_thread(s);
	attach(flat_ptr);
	dispatcher().assign("/flat/(\\d+)",&flat_thread::prepare,flat_ptr,1);
	mapper().assign("flat_thread","/flat/{1}");

	// Tree thread
	tree_thread *tree_ptr = new tree_thread(s);
	attach(tree_ptr);
	dispatcher().assign("/tree/(\\d+)",&tree_thread::prepare,tree_ptr,1);
	mapper().assign("tree_thread","/tree/{1}");
	
	// Generic mapping
	mapper().assign("user_thread","/{method}/{1}");
	
	// Reply
	reply *repl_ptr = new reply(s);
	attach(repl_ptr);
	dispatcher().assign("/comment/(\\d+)",&reply::prepare,repl_ptr,1);
	mapper().assign("comment","/comment/{1}");
}

} // apps
