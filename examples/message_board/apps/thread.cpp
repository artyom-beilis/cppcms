#include "thread.h"
#include "master.h"
#include <cppcms/util.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/session_interface.h>



namespace apps {

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


std::string thread_shared::text2html(std::string const &s)
{
	std::string tmp=cppcms::util::escape(s);
	std::string res;
	res.reserve(tmp.size());
	for(unsigned i=0;i<tmp.size();i++) {
		if(tmp[i]=='\n') {
			res+="<br />";
		}
		res+=tmp[i];
	}
	return res;
}


thread_shared::thread_shared(cppcms::service &s) : apps::master(s)
{
}


void thread_shared::clear()
{
	title.clear();
	thread_id = 0;
	master::clear();
}

bool thread_shared::prepare(int id)
{
	master::prepare();
	thread_id = id;

	cppdb::result r;
	r=sql<<"SELECT title FROM threads WHERE id=?" << id << cppdb::row;
	if(r.empty()) {
		response().status(404);
		return false;
	}
	
	r>> title;
	return true;

}

flat_thread::flat_thread(cppcms::service &s) : thread_shared(s)
{

}

void flat_thread::clear()
{
	messages.clear();
	thread_shared::clear();
}

void flat_thread::prepare(std::string sid)
{
	int id = atoi(sid.c_str());
	thread_shared::prepare(id);
	cppdb::result r;
	r=sql<<	"SELECT id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY id" << id;
	
	messages.reserve(10);
	for(int i=0;r.next();i++) {
		messages.resize(i+1);
		r>>messages[i].msg_id>>messages[i].author>>messages[i].content;
	}
	session()["view"]="flat";
	render("flat_thread",*this);
}

typedef std::map<int,std::map<int,msg> > msg_ord_t;

namespace {

void make_tree(tree_thread::tree_msg::tree_t &messages,std::map<int,std::map<int,msg> > &content,int start)
{
	std::pair<msg_ord_t::iterator,msg_ord_t::iterator>
		range=content.equal_range(start);
	for(msg_ord_t::iterator p=range.first;p!=range.second;++p) {
		for(std::map<int,msg>::iterator p2=p->second.begin(),e=p->second.end();p2!=e;++p2) {
			tree_thread::tree_msg &m=messages[p2->first];
			m.author=p2->second.author;
			m.content=p2->second.content;
			m.msg_id=p2->second.msg_id;
			make_tree(m.repl,content,p2->first);
		}
	}
	
}

} // anon

tree_thread::tree_thread(cppcms::service &s) : thread_shared(s)
{
}

void tree_thread::clear()
{
	messages.clear();
	thread_shared::clear();
}


void tree_thread::prepare(std::string sid)
{
	int id = atoi(sid.c_str());
	thread_shared::prepare(id);
	cppdb::result r;
	r=sql<<	"SELECT reply_to,id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY reply_to,id DESC" << id;
	msg_ord_t all;
	while(r.next()) {
		int msg_id,rpl_id;
		std::string author,comment;
		r>>rpl_id>>msg_id;
		msg &message=all[rpl_id][msg_id];
		r>>message.author>>message.content;
		message.msg_id=msg_id;
	}
	
	make_tree(messages,all,0);

	session()["view"]="tree";
	render("tree_thread",*this);
}

reply::reply(cppcms::service &srv) : thread_shared(srv)
{
}

void reply::clear()
{
	form.clear();
	thread_shared::clear();
}

void reply::prepare(std::string smid)
{
	int mid;
	mid=atoi(smid.c_str());

	thread_shared::prepare(mid);

	if(request().request_method()=="POST") {
		form.load(context());
		if(form.validate()) {
			cppdb::transaction tr(sql);
			cppdb::result r;
			r=sql<<"SELECT thread_id FROM messages WHERE id=?" << mid << cppdb::row;
			if(r.empty()) {
				response().status(404);
				return;
			}
			int tid;
			r>>tid;
			sql<<	"INSERT INTO messages(reply_to,thread_id,author,content) "
				"VALUES(?,?,?,?) " 
				<< mid << tid << form.author.value() << form.comment.value() 
				<< cppdb::exec;
			tr.commit();

			session()["author"]=form.author.value();

			response().set_redirect_header(url("/user_thread",tid));
			return;
		}
	}

	if(session().is_set("author")) {
		form.author.value(session()["author"]);
	}
	cppdb::result r;
	r=sql<<	"SELECT threads.id,author,content,title "
		"FROM messages "
		"JOIN threads ON thread_id=threads.id "
		"WHERE messages.id=?" << mid << cppdb::row;
	if(r.empty()) {
		response().set_redirect_header(url("/user_thread",mid));
		return;
	}

	int tid;

	r>>tid>>author>>content>>title;

	render("reply",*this);
}

} // namespace apps
