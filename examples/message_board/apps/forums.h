#ifndef FORUMS_H
#define FORUMS_H
#include <cppcms/application.h>

#include "master.h"

namespace apps {

	struct new_topic_form : public cppcms::form {
		cppcms::widgets::text title;
		cppcms::widgets::text author;
		cppcms::widgets::textarea comment;
		cppcms::widgets::submit submit;
		new_topic_form();
	};


	class forums : public master{
	public:
		struct topic {
			std::string title;
			int id;
		};
		std::vector<topic> topics;
		int next,prev;
		new_topic_form form;
		
		forums(cppcms::service &s);
		void prepare(std::string page);
		virtual void clear();
	};

}

#endif
