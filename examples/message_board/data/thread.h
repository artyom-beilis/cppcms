#ifndef DATA_THREAD_H
#define DATA_THREAD_H
#include <string>

#include <data/master.h>
#include <booster/function.h>

namespace data {

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
	thread_shared() 
	{
		thread_id = 0;
	}
	int thread_id;
	std::string title;
	std::string (*text2html)(std::string const &);

};

class flat_thread : public thread_shared {
public:
	std::vector<msg> messages;

};

class tree_thread : public thread_shared {
public:
	struct tree_msg : public msg {
		typedef std::map<int,tree_msg> tree_t;
		tree_t repl;
	};
	tree_msg::tree_t messages;

};

typedef tree_thread::tree_msg::tree_t tree_t;

class reply : public thread_shared , public msg {
public:
	reply_form form;

};


} // data


#endif
