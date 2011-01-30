#include "forums.h"
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/session_interface.h>
#include <boost/lexical_cast.hpp>

namespace data {

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

} // data

namespace apps {

forums::forums(cppcms::service &srv) : master(srv)
{
}



void forums::prepare(std::string page)
{
	const unsigned topics_per_page=10;
	data::forums c;
	master::prepare(c);
	if(request().request_method()=="POST") {
		c.form.load(context());
		if(c.form.validate()) {
			cppdb::transaction tr(sql);
			cppdb::statement st;
			st= sql	<<"INSERT INTO threads(title) VALUES(?)" 
				<< c.form.title.value() << cppdb::exec;
			int id=st.last_insert_id();
			sql<<	"INSERT INTO messages(thread_id,reply_to,content,author) "
				"VALUES (?,0,?,?)"
				<< id << c.form.comment.value() << c.form.author.value() << cppdb::exec;
			tr.commit();
			session()["author"]=c.form.author.value();
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
	c.topics.reserve(topics_per_page);	
	for(int i=0;r.next();i++) {
		c.topics.resize(c.topics.size()+1);
		r>>c.topics[i].id>>c.topics[i].title;
	}
	if(c.topics.size()==topics_per_page) {
		c.next=offset+1;
	}
	else {
		c.next = 0;
	}
	if(offset>0) {
		c.prev=offset-1;
	}
	else {
		c.prev = 0;
	}
	if(session().is_set("author")) {
		c.form.author.value(session()["author"]);
	}
	render("forums",c);
}


} // namespace apps
