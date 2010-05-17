//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#include <booster/log.h>
#include <booster/posix_time.h>
#include <booster/thread.h>
#include <booster/shared_ptr.h>
#include <booster/weak_ptr.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>
#include <set>
#include <string.h>

#ifdef BOOSTER_POSIX
#include <syslog.h>
#endif

namespace booster {
namespace log {
	struct message::data{};

	message::message() :
		level_(error),
		module_(""),
		file_name_(""),
		file_line_(1),
		message_(new std::ostringstream())
	{
		message_->imbue(std::locale::classic());
	}
	message::message(level_type l,char const *m,char const *name,int line) :
		level_(l),
		module_(m),
		file_name_(name),
		file_line_(line),
		message_(new std::ostringstream())
	{
		message_->imbue(std::locale::classic());
	}
	message::~message()
	{
		try {
			logger::instance().log(*this);
		}
		catch(...) {}
	}
	message::message(message &other) :
		level_(other.level_),
		module_(other.module_),
		file_name_(other.file_name_),
		file_line_(other.file_line_),
		message_(other.message_)
	{
	}
	message &message::operator=(message &other) 
	{
		if(this!=&other) {
			level_ = other.level_;
			module_ = other.module_;
			file_name_=other.file_name_;
			file_line_=other.file_line_;
			message_=other.message_;
		}
		return *this;
	}
	std::ostream &message::out() 
	{
		return *message_;
	}
	level_type message::level() const
	{
		return level_;
	}
	char const *message::module() const
	{
		return module_;
	}
	char const *message::file_name() const
	{
		return file_name_;
	}
	int message::file_line() const
	{
		return file_line_;
	}
	std::string message::log_message() const
	{
		return message_->str();
	}

	namespace {
		// make sure it is created
		struct init { init() { logger::instance(); } } the_init;
	}

	struct logger::data {
		mutex entries_lock;
		mutex log_lock;
		std::set<shared_ptr<sink> > sinks;
	};

	logger::logger() : d(new data())
	{
		memset(entries_,0,sizeof(entries_));
		default_level_ = error;
		entries_size_ = 0;
	}
	logger::~logger()
	{
	}
	
	bool logger::should_be_logged(level_type level,char const *module)
	{
		level_type module_level = default_level_;
		for(entry *p=entries_;p->module;p++) {
			if(strcmp(p->module,module)==0) {
				module_level=p->level;
				break;
			}
		}
		return level <= module_level;
	}

	void logger::set_log_level(level_type level,char const *module)
	{
		unique_lock<mutex> l(d->entries_lock);
		for(int i=0;i<entries_size_;i++) {
			if(strcmp(entries_[i].module,module)==0) {
				entries_[i].level=level;
				return;
			}
		}
		if(entries_size_ +1 >= max_entries_size_)
			return;
		entries_[entries_size_].module = module;
		entries_[entries_size_].level = level;
		entries_size_++;
	}

	void logger::reset_log_level(char const *module)
	{
		unique_lock<mutex> l(d->entries_lock);
		for(int i=0;i<entries_size_;i++) {
			if(strcmp(entries_[i].module,module)==0) {
				entries_[i]=entries_[entries_size_-1];
				entries_size_--;
				entries_[entries_size_].module = 0;
				return;
			}
		}
	}

	void logger::set_default_level(level_type level)
	{
		unique_lock<mutex> l(d->entries_lock);
		default_level_ = level;
	}

	logger &logger::instance()
	{
		static logger the_logger;
		return the_logger;
	}

	void logger::add_sink(shared_ptr<sink> const &s)
	{
		unique_lock<mutex> l(d->log_lock);
		d->sinks.insert(s);
	}
	void logger::remove_sink(weak_ptr<sink> const &ws)
	{
		unique_lock<mutex> l(d->log_lock);
		shared_ptr<sink> s=ws.lock();
		if(!s)
			return;
		d->sinks.erase(s);
	}
	void logger::remove_all_sinks()
	{
		unique_lock<mutex> l(d->log_lock);
		d->sinks.clear();
	}

	void logger::log(message const &m)
	{
		try {
			unique_lock<mutex> l(d->log_lock);
			std::set<shared_ptr<sink> >::iterator p;
			for(p=d->sinks.begin();p!=d->sinks.end();++p) {
				(*p)->log(m);
			}
		}
		catch(...) {}
	}

	char const *logger::level_to_string(level_type level)
	{
		switch(level) {
		case emergency : return "emergency";
		case alert: return "alert";
		case critical: return "critical";
		case error: return "error";
		case warning: return "warning";
		case notice: return "notice";
		case info: return "info";
		case debug: return "debug";
		default:
			return "unknown";
		}
	}

	namespace sinks {
		std::string format_plain_text_message(message const &msg)
		{
			std::ostringstream ss;
			ss.imbue(std::locale::classic());
			ptime now = ptime::now();
			std::tm formatted = ptime::universal_time(now);
			static char const format[]="%Y-%m-%d %H:%M:%S GMT; ";
			std::use_facet<std::time_put<char> >(ss.getloc()).put(ss,ss,' ',&formatted,format,format+sizeof(format)-1);
			ss << msg.module()<<", " << logger::level_to_string(msg.level()) << ": " << msg.log_message();

			ss <<" (" << msg.file_name() <<":" <<msg.file_line() <<")";
			return ss.str();
		}
		struct standard_error::data{};
		standard_error::standard_error()
		{
		}
		standard_error::~standard_error()
		{
		}
		void standard_error::log(message const &msg)
		{
			std::cerr << format_plain_text_message(msg) << std::endl;
		}

		struct file::data {};
		file::file() :
			file_(new std::fstream()),
			max_files_(0),
			max_size_(0),
			current_size_(0),
			opened_(false)
		{
			file_->imbue(std::locale::classic());
		}
		file::~file()
		{
		}
		
		/*
		void file::max_files(unsigned m)
		{
			if(!opened_) max_files_=m;
		}
		void file::max_size(size_t file_size)
		{
			if(!opened_) max_size_ = file_size;
		}
		*/

		void file::open(std::string file_name)
		{
			file_->open(file_name.c_str(),std::fstream::out);
			if(!*file_)
				throw std::runtime_error("Failed to open file " + file_name);
		}
		void file::log(message const &msg)
		{
			*file_ << format_plain_text_message(msg) << std::endl;
		}
		#ifdef BOOSTER_POSIX
		struct syslog::data {};
		syslog::syslog()
		{
		}
		syslog::~syslog()
		{
		}
		void syslog::log(message const &msg)
		{
			int syslevel;
			level_type l=msg.level();
			if(/*emergency <= l &&*/ l < alert)
				syslevel = LOG_EMERG;
			else if(alert <= l && l < critical)
				syslevel = LOG_ALERT;
			else if(critical <= l && l < error)
				syslevel = LOG_CRIT;
			else if(error <= l && l < warning)
				syslevel=LOG_ERR;
			else if(warning <= l && l < notice)
				syslevel = LOG_WARNING;
			else if(notice <= l && l< info)
				syslevel = LOG_NOTICE;
			else if(info <= l && l<debug)
				syslevel = LOG_INFO;
			else /*if(debug <= l)*/
				syslevel = LOG_DEBUG;
			::syslog(syslevel,"%s: %s (%s:%d)",msg.module(),msg.log_message().c_str(),msg.file_name(),msg.file_line());
		}
		#endif
	}

		
}// log
}// booster
