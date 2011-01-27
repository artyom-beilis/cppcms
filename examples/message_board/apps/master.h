#ifndef MASTER_H
#define MASTER_H
#include <cppcms/application.h>
#include <cppcms/view.h>
#include <cppdb/frontend.h>

namespace apps {

class master: public cppcms::application, public cppcms::base_content {
public:
	std::string media;

	master(cppcms::service &w);
	virtual void init();
	virtual void clear();
protected:
	void prepare();
	cppdb::session sql;
};


}


#endif
