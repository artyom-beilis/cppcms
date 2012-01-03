///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_IMPL_WINSERVICE_H
#define CPPCMS_IMPL_WINSERVICE_H

#include <booster/function.h>
#include <cppcms/defs.h>
#include <cppcms/json.h>
#include <string>

namespace cppcms {
namespace impl {
	
	class CPPCMS_API winservice {
		winservice();
		~winservice();
	public:
		typedef booster::function<void()> callback_type;
		
		static winservice &instance();
		// Returns false if it was only install/uninstall process
		// rather then execution
		void prepare(callback_type const &callback)
		{
			prepare_=callback;
		}
		void prepare()
		{
			if(prepare_) prepare_();
		}
		void stop(callback_type const &callback)
		{
			stop_ = callback;
		}
		void stop()
		{
			if(stop_) stop_();
		}
		void exec(callback_type const &callback)
		{
			exec_ = callback;
		}
		void exec()
		{
			if(exec_) exec_();
		}
		
		void run(json::value &conf,int argc,char **argv);
		
		static bool is_console(json::value &conf)
		{
			return conf.get("winservice.mode","console") == "console";
		}
	
	private:
		callback_type prepare_,stop_,exec_;
		std::vector<std::string> args_;
		void uninstall();
		void install();
		void service();
		void console();
		json::value settings_;
	};

} 
}


#endif
