#ifndef FORUMS_CONTENT_H
#define FORUMS_CONTENT_H
#include "master_content.h"
#include <cppcms/form.h>

namespace cppcms { class application; }

namespace content {

struct new_topic_form : public cppcms::form {
	widgets::text title;
	widgets::text author;
	widgets::textarea comment;
	widgets::submit submit;
	new_topic_form();
};

struct forums : public master {
	struct topic {
		std::string title;
		std::string url;
	};
	std::vector<topic> topics;
	std::string next_page,prev_page;
	new_topic_form form;
};

};


#endif
