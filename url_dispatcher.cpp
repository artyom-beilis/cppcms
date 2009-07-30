#define CPPCMS_SOURCE
#include "url_dispatcher.h"
#include "application.h"

#include <boost/regex.hpp>


namespace cppcms {

	namespace /* anon */ {
		struct option : public util::noncopyable {
			option(std::string expr) :
				expr_(expr)
			{
			}
			virtual ~option()
			{
			}

			bool matches(std::string path)
			{
				return boost::regex_match(path.c_str(),match_,expr_);
			}

			virtual void dispatch() = 0;
			virtual dispatch_type dispatchable(std::string path) = 0;



		protected:
			boost::regex expr_;
			boost::cmatch match_;
		};

		struct mounted : public option {
			mounted(std::string expr,int select,application *app) :
				option(expr),
				select_(p),
				app_(app)
			{
			}

			virtual dispatch_type dispatchable()
			{
				return app_->dispatcher(match_[select_]);
			}
			virtual void dispatch()
			{
				app_->dispatch();
			}
		private:
			application *app_;
			int select_;
		};

		template<typename H>
		struct base_handler : public option {
			base_handler(std::string expr,H handle,bool async,int a=0,int b=0,int c=0,int d=0)
				: option(expr),async_(async),handle_(handle)
			{
				select_[0]=a;
				select_[1]=b;
				select_[2]=c;
				select_[3]=d;
			}
			virtual dispatch_type dispatchable()
			{
				return async_ ? url_dispatcher::asynchronous : url_dispatcher::synchronous;
			}
			virtual void dispatch()
			{
				execute_handler(handle_,select_,match_);
			}
		private:
			void execute_handler(url_dispatcher::handler const &h)
			{
				h();
			}

			void execute_handler(url_dispatcher::handler1 const &h)
			{
				h(match_[select_[0]]);
			}

			void execute_handler(url_dispatcher::handler2 const &h)
			{
				h(match_[select_[0]],match_[select_[1]]);
			}
			void execute_handler(url_dispatcher::handler3 const &h)
			{
				h(match_[select_[0]],match_[select_[1]],match_[select_[2]]);
			}
			void execute_handler(url_dispatcher::handler4 const &h)
			{
				h(match_[select_[0]],match_[select_[1]],match_[select_[2]],match_[select_[3]]);
			}

			bool async_;
			int select_[4];
			H handle_;
		};


		template<typename H>
		boost::shared_ptr<option> make_handler(std::string expr,H const &handler,bool async,int a=0,int b=0,int c=0,int d=0)
		{
			return boost::shared_ptr<option>(new base_handler<H>(expr,handler,async,a,b,c,d));
		}

	} // anonynoys


	struct url_dispatcher::data {
		std::vector<boost::shared_ptr<option> > options_;
		boost::shared_ptr<option> last_option_;
	};

	// Meanwhile nothing
	url_dispatcher::url_dispatcher() :
			d(new url_dispatcher::data())
	{
	}
	url_dispatcher::~url_dispatcher()
	{
	}

	url_dispatcher::dispatch_type url_dispatcher::dispatchable(std::string path)
	{
		unsigned i;
		for(i=0;i<d->options.size();i++) {
			if(d->options[i]->match())
				d->last_option_=d->options[i];
			return d->last_option_->dispatchable();
		}
		return none;
	}

	bool url_dispatcher::dispatch()
	{
		if(d->last_option_)
			d->last_option_->dispatch();
		d->last_option_.reset();
	}

	void url_dispatcher::mount(std::string match,application &app,int select)
	{
		d->options_.push_back(boost::shared_ptr<option>(new mounted(match,&app,select)));
	}



	void url_dispatcher::assign(std::string expr,handler h)
	{
		d->options_.push_back(make_handler(expr,h,false));
	}

	void url_dispatcher::assign(std::string expr,handler1 h,int a)
	{
		d->options_.push_back(make_handler(expr,h,false,a));
	}

	void url_dispatcher::assign(std::string expr,handler2 h,int a,int b)
	{
		d->options_.push_back(make_handler(expr,h,false,a,b));
	}

	void url_dispatcher::assign(std::string expr,handler3 h,int a,int b,int c)
	{
		d->options_.push_back(make_handler(expr,h,false,a,b,c));
	}

	void url_dispatcher::assign(std::string expr,handler4 h,int a,int b,int c,int d)
	{
		d->options_.push_back(make_handler(expr,h,false,a,b,c,d));
	}

} // namespace cppcms
