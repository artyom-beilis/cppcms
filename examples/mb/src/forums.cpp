#include "forums.h"
#include "forums_content.h"
#include "mb.h"
#include <cppcms/url_dispatcher.h>
#include <cppcms/session_interface.h>
#include <boost/lexical_cast.hpp>

using namespace dbixx;

namespace content {

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

} // content

namespace apps {

forums::forums(mb &b) :
	application(b.service()),
	board(b)
{
	board.dispatcher().assign("^(/(\\w+)?)?$",&forums::display_forums,this,2);
}

string forums::forums_url(int offset)
{
	string link=request().script_name();
	if(offset==0)
		return link;
	link.append("/");
	link+=boost::lexical_cast<string>(offset);
	return link;
}

void forums::display_forums(string page)
{
	const unsigned topics_per_page=10;
	content::forums c;
	board.ini(c);
	if(request().request_method()=="POST") {
		c.form.load(context());
		if(c.form.validate()) {
			dbixx::transaction tr(board.sql);
			board.sql<<
				"INSERT INTO threads(title) VALUES(?)",
				c.form.title.value(),exec();
			int id=board.sql.rowid();
			board.sql<<
				"INSERT INTO messages(thread_id,reply_to,content,author) "
				"VALUES (?,0,?,?)",
				id,c.form.comment.value(),c.form.author.value(),exec();
			tr.commit();
			session()["author"]=c.form.author.value();
			response().set_redirect_header(board.thread.user_url(id));
			return;
		}
	}
	int offset= page.empty() ? 0 : atoi(page.c_str());
	dbixx::result res;
	board.sql<<
		"SELECT id,title "
		"FROM threads "
		"ORDER BY id DESC "
		"LIMIT ?,?",offset*topics_per_page,topics_per_page,res;
	c.topics.resize(res.rows());
	dbixx::row r;
	for(int i=0;res.next(r);i++) {
		int id;
		r>>id>>c.topics[i].title;
		c.topics[i].url=board.thread.user_url(id);
	}
	if(c.topics.size()==topics_per_page) {
		c.next_page=forums_url(offset+1);
	}
	if(offset>0) {
		c.prev_page=forums_url(offset-1);
	}
	if(session().is_set("author")) {
		c.form.author.value(session()["author"]);
	}
	render("forums",c);
}


} // namespace apps
