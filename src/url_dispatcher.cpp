///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/url_dispatcher.h>
#include <cppcms/application.h>

#include <booster/regex.h>
#include <booster/shared_ptr.h>

namespace cppcms {

	namespace /* anon */ {
		struct option : public booster::noncopyable {
			option(std::string expr) :
				expr_(expr)
			{
			}
			virtual ~option()
			{
			}

			bool matches(std::string const &path)
			{
				return booster::regex_match(path.c_str(),match_,expr_);
			}

			virtual bool dispatch(std::string url) = 0;
		protected:
			booster::regex expr_;
			booster::cmatch match_;
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
		booster::shared_ptr<option> make_handler(std::string expr,H const &handler,int a=0,int b=0,int c=0,int d=0)
		{
			return booster::shared_ptr<option>(new base_handler<H>(expr,handler,a,b,c,d));
		}

	} // anonynoys


	struct url_dispatcher::_data {
		std::vector<booster::shared_ptr<option> > options;
		booster::shared_ptr<option> last_option;
	};

	// Meanwhile nothing
	url_dispatcher::url_dispatcher() :
			d(new url_dispatcher::_data())
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
		d->options.push_back(booster::shared_ptr<option>(new mounted(match,select,&app)));
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
