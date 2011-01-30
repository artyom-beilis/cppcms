#include <apps/thread.h>
#include <cppcms/util.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/url_mapper.h>
#include <cppcms/session_interface.h>



namespace data {

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


} // data

namespace {
	std::string text2html(std::string const &s)
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
}


namespace apps {

thread_shared::thread_shared(cppcms::service &s) : apps::master(s)
{
}


bool thread_shared::prepare(data::thread_shared &c,int id)
{
	master::prepare(c);
	c.thread_id = id;
	

	cppdb::result r;
	r=sql<<"SELECT title FROM threads WHERE id=?" << id << cppdb::row;
	if(r.empty()) {
		response().make_error_response(404);
		return false;
	}
	
	r>> c.title;
	c.text2html = text2html;
	return true;

}

flat_thread::flat_thread(cppcms::service &s) : thread_shared(s)
{

}


void flat_thread::prepare(std::string sid)
{
	data::flat_thread c;
	int id = atoi(sid.c_str());
	if(!thread_shared::prepare(c,id))
		return;
	cppdb::result r;
	r=sql<<	"SELECT id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY id" << id;
	
	c.messages.reserve(10);
	for(int i=0;r.next();i++) {
		c.messages.resize(i+1);
		r>>c.messages[i].msg_id>>c.messages[i].author>>c.messages[i].content;
	}
	session()["view"]="flat";
	render("flat_thread",c);
}

typedef std::map<int,std::map<int,data::msg> > msg_ord_t;

namespace {

void make_tree(data::tree_t &messages,std::map<int,std::map<int,data::msg> > &content,int start)
{
	std::pair<msg_ord_t::iterator,msg_ord_t::iterator>
		range=content.equal_range(start);
	for(msg_ord_t::iterator p=range.first;p!=range.second;++p) {
		for(std::map<int,data::msg>::iterator p2=p->second.begin(),e=p->second.end();p2!=e;++p2) {
			data::tree_thread::tree_msg &m=messages[p2->first];
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



void tree_thread::prepare(std::string sid)
{
	int id = atoi(sid.c_str());
	data::tree_thread c;
	if(!thread_shared::prepare(c,id))
		return;
	cppdb::result r;
	r=sql<<	"SELECT reply_to,id,author,content "
		"FROM messages WHERE thread_id=? "
		"ORDER BY reply_to,id DESC" << id;
	msg_ord_t all;
	while(r.next()) {
		int msg_id,rpl_id;
		std::string author,comment;
		r>>rpl_id>>msg_id;
		data::msg &message=all[rpl_id][msg_id];
		r>>message.author>>message.content;
		message.msg_id=msg_id;
	}
	
	make_tree(c.messages,all,0);

	session()["view"]="tree";
	render("tree_thread",c);
}

reply::reply(cppcms::service &srv) : thread_shared(srv)
{
}

void reply::prepare(std::string smid)
{
	int mid;
	mid=atoi(smid.c_str());

	data::reply c;
	
	if(request().request_method()=="POST") {
		c.form.load(context());
		if(c.form.validate()) {
			cppdb::transaction tr(sql);
			cppdb::result r;
			r=sql<<"SELECT thread_id FROM messages WHERE id=?" << mid << cppdb::row;
			if(r.empty()) {
				response().make_error_response(404);
				return;
			}
			int tid;
			r>>tid;
			r.clear();
			sql<<	"INSERT INTO messages(reply_to,thread_id,author,content) "
				"VALUES(?,?,?,?) " 
				<< mid << tid << c.form.author.value() << c.form.comment.value() 
				<< cppdb::exec;
			tr.commit();

			session()["author"]=c.form.author.value();

			response().set_redirect_header(url("/user_thread",tid));
			return;
		}
	}

	if(session().is_set("author")) {
		c.form.author.value(session()["author"]);
	}
	cppdb::result r;
	r=sql<<	"SELECT threads.id,author,content,title "
		"FROM messages "
		"JOIN threads ON thread_id=threads.id "
		"WHERE messages.id=?" << mid << cppdb::row;
	if(r.empty()) {
		response().make_error_response(404);
		return;
	}

	int tid;

	r>>tid>>c.author>>c.content>>c.title;
	
	if(!thread_shared::prepare(c,tid))
		return;

	render("reply",c);
}

} // namespace apps
