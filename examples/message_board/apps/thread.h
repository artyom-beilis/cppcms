#ifndef THREAD_H
#define THREAD_H
#include <cppcms/application.h>
#include <string>

#include "master.h"

namespace apps {

struct reply_form : public cppcms::form {
	cppcms::widgets::text author;
	cppcms::widgets::textarea comment;
	cppcms::widgets::submit send;
	reply_form();
};


struct msg {
	std::string author;
	std::string content;
	int msg_id;
};


class thread_shared : public master
{
public:
	
	int thread_id;
	std::string title;

	std::string text2html(std::string const &s);

	thread_shared(cppcms::service &s);
	bool prepare(int id);
	virtual void clear();

};

class flat_thread : public thread_shared {
public:
	std::vector<msg> messages;

	flat_thread(cppcms::service &s);
	void prepare(std::string id); 
	virtual void clear();
};

class tree_thread : public thread_shared {
public:
	struct tree_msg : public msg {
		typedef std::map<int,tree_msg> tree_t;
		tree_t repl;
	};
	tree_msg::tree_t messages;

	tree_thread(cppcms::service &s);
	void prepare(std::string id);
	virtual void clear();
};

typedef tree_thread::tree_msg::tree_t tree_t;

class reply : public thread_shared , public msg {
public:
	reply_form form;

	reply(cppcms::service &s);
	void prepare(std::string id);
	virtual void clear();
};


} // apps


#endif
