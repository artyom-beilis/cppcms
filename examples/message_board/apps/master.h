#ifndef APPS_MASTER_H
#define APPS_MASTER_H
#include <cppcms/application.h>
#include <cppdb/frontend.h>

#include <data/master.h>

namespace apps {

class master: public cppcms::application {
public:

	master(cppcms::service &w);
	virtual void init();
	virtual void clear();

protected:
	void prepare(data::master &c);
	cppdb::session sql;
private:
	std::string conn_str_;
};


}


#endif
