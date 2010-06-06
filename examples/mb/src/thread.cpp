#include "thread.h"
#include "thread_content.h"
#include "mb.h"
#include <cppcms/util.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/session_interface.h>
#include <boost/lexical_cast.hpp>

using boost::lexical_cast;

namespace content {

reply_form::reply_form()
{
	using cppcms::locale::translate;
	author.message(translate("Author"));
	comment.message(translate("Comment"));
	send.value(translate("Send"));
	*this + author + comment + send;
	author.limits(1,64);
	comment.limits(1,256);
}


string thread_shared::text2html(string const &s)
{
	string tmp=cppcms::util::escape(s);
	string res;
	res.reserve(tmp.size());
	for(unsigned i=0;i<tmp.size();i++) {
		if(tmp[i]=='\n') {
			res+="<br />";
		}
		res+=tmp[i];
	}
	return res;
}


} // namespace content

namespace apps {

thread::thread(mb &b) : application(b.service()) , board(b) 
{
	board.dispatcher().assign("^/flat/(\\d+)/?$",&thread::flat,this,1);
	board.dispatcher().assign("^/tree/(\\d+)/?$",&thread::tree,this,1);
	board.dispatcher().assign("^/comment/(\\d+)/?$",&thread::reply,this,1);
}

string thread::flat_url(int id)
{
	return request().script_name()+"/flat/"+lexical_cast<string>(id);
}

string thread::tree_url(int id)
{
	return request().script_name()+"/tree/"+lexical_cast<string>(id);
}

string thread::user_url(int id)
{
	if(!session().is_set("view") || session()["view"]=="tree") {
		return tree_url(id);
	}
	return flat_url(id);
}

string thread::reply_url(int message_id)
{
	string tmp=request().script_name();
	tmp+="/comment/";
	tmp+=lexical_cast<string>(message_id);
	return tmp;
}
int thread::ini(string sid,content::base_thread &c)
{
	int id=lexical_cast<int>(sid);
	board.sql<<"SELECT title FROM threads WHERE id=?",id;
	dbixx::row r;
	if(!board.sql.single(r)) {
		throw e404();
	}
	board.ini(c);
	r>>c.title;
	c.flat_view=flat_url(id);
	c.tree_view=tree_url(id);
	return id;
}

void thread::flat(string sid)
{
	content::flat_thread c;
	int id=ini(sid,c);
	board.sql<<
		"SELECT id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY id",
		id;
	dbixx::result res;
	dbixx::row r;
	board.sql.fetch(res);
	c.messages.resize(res.rows());
	int i;
	for(i=0;res.next(r);i++) {
		int msg_id;
		r>>msg_id>>c.messages[i].author>>c.messages[i].content;
		c.messages[i].reply_url=reply_url(msg_id);
	}
	session()["view"]="flat";
	render("flat_thread",c);
}

typedef map<int,map<int,content::msg> > msg_ord_t;

namespace {

void make_tree(content::tree_t &messages,map<int,map<int,content::msg> > &content,int start)
{
	std::pair<msg_ord_t::iterator,msg_ord_t::iterator>
		range=content.equal_range(start);
	for(msg_ord_t::iterator p=range.first;p!=range.second;++p) {
		for(map<int,content::msg>::iterator p2=p->second.begin(),e=p->second.end();p2!=e;++p2) {
			content::tree_thread::tree_msg &m=messages[p2->first];
			m.author=p2->second.author;
			m.content=p2->second.content;
			m.reply_url=p2->second.reply_url;
			make_tree(m.repl,content,p2->first);
		}
	}
	
}

}

void thread::tree(string sid)
{
	content::tree_thread c;
	int id=ini(sid,c);
	board.sql<<
		"SELECT reply_to,id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY reply_to,id DESC",
		id;
	dbixx::result res;
	dbixx::row r;
	board.sql.fetch(res);
	msg_ord_t all;
	while(res.next(r)) {
		int msg_id,rpl_id;
		string author,comment;
		r>>rpl_id>>msg_id;
		content::msg &message=all[rpl_id][msg_id];
		r>>message.author>>message.content;
		message.reply_url=reply_url(msg_id);
	}
	
	make_tree(c.messages,all,0);

	session()["view"]="tree";
	render("tree_thread",c);
}

void thread::reply(string smid)
{
	int mid;
	mid=lexical_cast<int>(smid);

	content::reply c;

	if(request().request_method()=="POST") {
		c.form.load(context());
		if(c.form.validate()) {
			dbixx::transaction tr(board.sql);
			dbixx::row r;
			board.sql<<"SELECT thread_id FROM messages WHERE id=?",mid;
			if(!board.sql.single(r))
				throw e404();
			int tid;
			r>>tid;
			board.sql<<
				"INSERT INTO messages(reply_to,thread_id,author,content) "
				"VALUES(?,?,?,?) ",
				mid,tid,c.form.author.value(),c.form.comment.value();
			board.sql.exec();
			tr.commit();

			session()["author"]=c.form.author.value();

			response().set_redirect_header(user_url(tid));
			return;
		}
	}

	board.ini(c);
	if(session().is_set("author")) {
		c.form.author.value(session()["author"]);
	}
	dbixx::row r;
	board.sql<<
		"SELECT threads.id,author,content,title "
		"FROM messages "
		"JOIN threads ON thread_id=threads.id "
		"WHERE messages.id=?",
		mid;
	if(!board.sql.single(r)) {
		throw e404();
	}

	int tid;

	r>>tid>>c.author>>c.content>>c.title;

	c.back=user_url(tid);

	render("reply",c);
}

} // namespace apps
