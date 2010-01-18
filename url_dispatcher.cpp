#define CPPCMS_SOURCE
#include "url_dispatcher.h"
#include "application.h"

#include "config.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/regex.hpp>
#else // Internal Boost
#   include <cppcms_boost/regex.hpp>
    namespace boost = cppcms_boost;
#endif


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

			virtual bool dispatch(std::string url) = 0;
		protected:
			boost::regex expr_;
			boost::cmatch match_;
		};

		struct mounted : public option {
			mounted(std::string expr,int select,application *app) :
				option(expr),
				app_(app),
				select_(select)
			{
			}

			virtual bool dispatch(std::string url)
			{
				if(matches(url))
					return app_->dispatcher().dispatch(match_[select_]);
				return false;
			}
		private:
			application *app_;
			int select_;
		};

		template<typename H>
		struct base_handler : public option {
			base_handler(std::string expr,H handle,int a=0,int b=0,int c=0,int d=0)
				: option(expr),handle_(handle)
			{
				select_[0]=a;
				select_[1]=b;
				select_[2]=c;
				select_[3]=d;
			}
			virtual bool dispatch(std::string url)
			{
				if(matches(url)) {
					execute_handler(handle_);
					return true;
				}
				return false;
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

			int select_[4];
			H handle_;
		};


		template<typename H>
		boost::shared_ptr<option> make_handler(std::string expr,H const &handler,int a=0,int b=0,int c=0,int d=0)
		{
			return boost::shared_ptr<option>(new base_handler<H>(expr,handler,a,b,c,d));
		}

	} // anonynoys


	struct url_dispatcher::data {
		std::vector<boost::shared_ptr<option> > options;
		boost::shared_ptr<option> last_option;
	};

	// Meanwhile nothing
	url_dispatcher::url_dispatcher() :
			d(new url_dispatcher::data())
	{
	}
	url_dispatcher::~url_dispatcher()
	{
	}

	bool url_dispatcher::dispatch(std::string url)
	{
		unsigned i;
		for(i=0;i<d->options.size();i++) {
			if(d->options[i]->dispatch(url))
				return true;
		}
		return false;
	}

	void url_dispatcher::mount(std::string match,application &app,int select)
	{
		d->options.push_back(boost::shared_ptr<option>(new mounted(match,select,&app)));
	}



	void url_dispatcher::assign(std::string expr,handler h)
	{
		d->options.push_back(make_handler(expr,h));
	}

	void url_dispatcher::assign(std::string expr,handler1 h,int p1)
	{
		d->options.push_back(make_handler(expr,h,p1));
	}

	void url_dispatcher::assign(std::string expr,handler2 h,int p1,int p2)
	{
		d->options.push_back(make_handler(expr,h,p1,p2));
	}

	void url_dispatcher::assign(std::string expr,handler3 h,int p1,int p2,int p3)
	{
		d->options.push_back(make_handler(expr,h,p1,p2,p3));
	}

	void url_dispatcher::assign(std::string expr,handler4 h,int p1,int p2,int p3,int p4)
	{
		d->options.push_back(make_handler(expr,h,p1,p2,p3,p4));
	}

} // namespace cppcms
