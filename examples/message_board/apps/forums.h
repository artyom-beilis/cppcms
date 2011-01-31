#ifndef APPS_FORUMS_H
#define APPS_FORUMS_H
#include <cppcms/application.h>

#include <apps/master.h>
#include <data/forums.h>

namespace apps {


	class forums : public master{
	public:
		
		forums(cppcms::service &s);
		void prepare(std::string page);
	private:
		void prepare_content(data::forums &c,std::string const &page);
	};

}

#endif
