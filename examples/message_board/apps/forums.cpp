#include "forums.h"
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/session_interface.h>
#include <boost/lexical_cast.hpp>

namespace apps {

new_topic_form::new_topic_form() 
{
	using cppcms::locale::translate;
	title.message(translate("Title"));
	author.message(translate("Author"));
	comment.message(translate("Comment"));
	submit.value(translate("Create"));
	*this + title + author + comment + submit;
	title.non_empty();
	author.limits(1,64);
	comment.limits(1,256);
}

forums::forums(cppcms::service &srv) : master(srv)
{
}


void forums::clear()
{
	topics.clear();
	next = 0;
	prev = 0;
	form.clear();	
	master::clear();
}

void forums::prepare(std::string page)
{
	const unsigned topics_per_page=10;
	master::prepare();
	if(request().request_method()=="POST") {
		form.load(context());
		if(form.validate()) {
			cppdb::transaction tr(sql);
			cppdb::statement st;
			st= sql	<<"INSERT INTO threads(title) VALUES(?)" 
				<< form.title.value() << cppdb::exec;
			int id=st.last_insert_id();
			sql<<	"INSERT INTO messages(thread_id,reply_to,content,author) "
				"VALUES (?,0,?,?)"
				<< id << form.comment.value() << form.author.value() << cppdb::exec;
			tr.commit();
			session()["author"]=form.author.value();
			response().set_redirect_header(url("/user_thread",id));
			return;
		}
	}
	int offset= page.empty() ? 0 : atoi(page.c_str());
	cppdb::result r;
	r=sql<<	"SELECT id,title "
		"FROM threads "
		"ORDER BY id DESC "
		"LIMIT ?,?" << offset*topics_per_page << topics_per_page;
	topics.reserve(topics_per_page);	
	for(int i=0;r.next();i++) {
		topics.resize(topics.size()+1);
		r>>topics[i].id>>topics[i].title;
	}
	if(topics.size()==topics_per_page) {
		next=offset+1;
	}
	else {
		next = 0;
	}
	if(offset>0) {
		prev=offset-1;
	}
	else {
		prev = 0;
	}
	if(session().is_set("author")) {
		form.author.value(session()["author"]);
	}
	render("forums",*this);
}


} // namespace apps
