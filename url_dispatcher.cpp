#define CPPCMS_SOURCE
#include "url_dispatcher.h"
#include "application.h"

namespace cppcms {
	struct url_dispatcher::mounted {
		std::string prefix;
		util::regex const *expr;
		application *app;
		mounted() : expr(0), app(0) {}
	};

	struct url_dispatcher::option {
		url_dispatcher::handler0 h0;
		url_dispatcher::handler1 h1;
		url_dispatcher::handler2 h2;
		url_dispatcher::handler3 h3;
		url_dispatcher::handler4 h4;
		util::regex const *expr;
		application *app;
		int handler_no;
		int params[4];
		
		option() : 
			expr(0),
			app(0),
			handler_no(-1),
			params(0)
		{
		}
	};

	// Meanwhile nothing
	url_dispatcher() {}
	~url_dispatcher() {}

	bool url_dispatcher::dispatch(std::string path)
	{
		std::list<option>::iterator p;
		for(p=options_.begin();p!=options_.end();++p) {
			util::regex::result res;
			if(p->expr && p->expr->match(path,res)) {
				if(p->app) {
					return p->app->run(path);
				}
				int *params=p->params;
				switch(p->handler_no) {
				case 0:	p->h0();
					break;
				case 1: p->h1(res[params[0]]);
					break;
				case 2: p->h2(res[params[0]],res[params[1]]);
					break;
				case 3: p->h3(res[params[0]],res[params[1]],res[params[2]]);
					break;
				case 4: p->h4(res[params[0]],res[params[1]],res[params[2]],res[params[3]]);
					break;
				}
				return true;
			}
		}
		return false;
	}

	void url_dispatcher::mount(util::regex const &match,application &app)
	{
		options_.push_back(option);
		option &last=options_.back();
		last.expr=&match;
		last.app=&app;
	}
	void url_dispatcher::assign(util::regex const &match,handler h)
	{
		options_.push_back(option);
		option &last=options_.back();
		last.expr=&match;
		last.handler_no=0;
		last.h0=h;
	}
	void url_dispatcher::assign(util::regex const &match,handler1 h,int exp1)
	{
		options_.push_back(option);
		option &last=options_.back();
		last.expr=&match;
		last.handler_no=1;
		last.h1.swap(h);
		last.params[0]=exp1;
	}
	void url_dispatcher::assign(util::regex const &match,handler2 h,int exp1,int exp2)
	{
		options_.push_back(option);
		option &last=options_.back();
		last.expr=&match;
		last.handler_no=2;
		last.h2.swap(h);
		last.params[0]=exp1;
		last.params[1]=exp2;
	}
	void url_dispatcher::assign(util::regex const &match,handler3 h,int exp1,int exp2,int exp3)
	{
		options_.push_back(option);
		option &last=options_.back();
		last.expr=&match;
		last.handler_no=3;
		last.h4.swap(h);
		last.params[0]=exp1;
		last.params[1]=exp2;
		last.params[2]=exp3;
	}
	void url_dispatcher::assign(util::regex const &match,handler4 h,int exp1,int exp2,int exp3,int exp4)
	{
		options_.push_back(option);
		option &last=options_.back();
		last.expr=&match;
		last.handler_no=4;
		last.h4.swap(h);
		last.params[0]=exp1;
		last.params[1]=exp2;
		last.params[2]=exp3;
		last.params[3]=exp3;
	}



} // namespace cppcms 
