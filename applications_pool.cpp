#define CPPCMS_SOURCE
#include "applications_pool.h"
#include "application.h"
#include <set>
#include <vector>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>

namespace cppcms {

	namespace {
		struct app_data : public util::noncopyable {
			app_data(std::string e,std::auto_ptr<applications_pool::factory> f,int m) :
				expr(e),
				match(m),
				factory(f),
				size(0)
			{
			}
			boost::regex expr;
			int match;
			std::auto_ptr<applications_pool::factory> factory;

			int size;
			std::set<application *> pool;

			~app_data()
			{
				std::set<application *>::iterator p;
				for(p=pool.begin();p!=pool.end();++p) {
					delete *p;
				}
			}
		};

	}

	struct applications_pool::data {
		std::vector<boost::shared_ptr<app_data> > apps;
		int limit;
	};


applications_pool::applications_pool(service &srv,int limit) :
	srv_(&srv),
	d(new applications_pool::data())
{
	d->limit=limit;
}
applications_pool::~applications_pool()
{
}

void applications_pool::mount(std::string pattern,std::auto_ptr<factory> aps,int select)
{
	d->apps.push_back(boost::shared_ptr<app_data>(new app_data(pattern,aps,select)));
}

std::auto_ptr<application> applications_pool::get(std::string path,std::string &matched)
{
	for(unsigned i=0;i<d->apps.size();i++) {
		boost::cmatch match;
		if(boost::regex_match(path.c_str(),match,d->apps[i]->expr)) {
			matched=match[d->apps[i]->match];
			if(d->apps[i]->pool.empty()) {
				std::auto_ptr<application> app=(*d->apps[i]->factory)(*srv_);
				app->pool_id(i);
				return app;
			}
			d->apps[i]->size--;
			std::auto_ptr<application> app(*(d->apps[i]->pool.begin()));
			d->apps[i]->pool.erase(app.get());
			return app;
		}
	}
	return std::auto_ptr<application>();
}

void applications_pool::put(std::auto_ptr<application> app)
{
	unsigned id=app->pool_id();
	if(id >= d->apps.size() || d->apps[id]->size >= d->limit)
		return;
	d->apps[id]->pool.insert(app.release());
	d->apps[id]->size++;
}


} //cppcms
