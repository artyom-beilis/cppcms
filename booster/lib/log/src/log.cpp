//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
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
#include <booster/nowide/fstream.h>
#include <booster/nowide/cstdio.h>
#include "../../locale/src/util/timezone.h"
#include <locale>
#include <set>
#include <utility>
#include <string.h>

#include <stdio.h>

#include <booster/backtrace.h>

#ifdef BOOSTER_POSIX
#include <syslog.h>
#include <unistd.h>
#else
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
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
		message_(std::move(other.message_))
	{
	}
	message &message::operator=(message &other) 
	{
		if(this!=&other) {
			level_ = other.level_;
			module_ = other.module_;
			file_name_=other.file_name_;
			file_line_=other.file_line_;
			message_=std::move(other.message_);
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
		char const *begin = file_name_;
		char const *p = begin + strlen(file_name_);
		for(;;) {
			switch(*p) {
			case '/':	// posix directory separator
			case '\\':	// windows directory separator 
			case ':':	// windows and vms disk separator
			case ']':	// vms directory separator
				return p+1;
			default:
				if(p == begin)
					return p;
				else
					p--;
			}
		} 
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
		struct init { 
			init() 
			{
				logger::instance();
			} 
		} the_init;
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

	level_type logger::string_to_level(std::string const &l)
	{
		if(l=="emergency") return emergency;
		if(l=="alert") return alert;
		if(l=="critical") return critical;
		if(l=="error") return error;
		if(l=="warning") return warning;
		if(l=="notice") return notice;
		if(l=="info") return info;
		if(l=="debug") return debug;
		throw booster::invalid_argument("Invalig logging level :" + l);
	}

	namespace sinks {
		std::string format_plain_text_message(message const &msg)
		{
			std::ostringstream ss;
			ss.imbue(std::locale::classic());
			ptime now = ptime::now();
			std::tm formatted = ptime::local_time(now);
			static char const format[]="%Y-%m-%d %H:%M:%S; ";
			std::use_facet<std::time_put<char> >(ss.getloc()).put(ss,ss,' ',&formatted,format,format+sizeof(format)-1);
			ss << msg.module()<<", " << logger::level_to_string(msg.level()) << ": " << msg.log_message();

			ss <<" (" << msg.file_name() <<":" <<msg.file_line() <<")";
			return ss.str();
		}

		std::string format_plain_text_message_tz(message const &msg,int tz_offset)
		{
			std::ostringstream ss;
			ss.imbue(std::locale::classic());
			ptime now = ptime::now() + ptime::from_number(tz_offset);
			std::tm formatted = ptime::universal_time(now);
			static char const format[]="%Y-%m-%d %H:%M:%S";
			std::use_facet<std::time_put<char> >(ss.getloc()).put(ss,ss,' ',&formatted,format,format+sizeof(format)-1);
			ss << " GMT";
			if(tz_offset != 0) {
				char sign = tz_offset > 0 ? '+' : '-';
				if(tz_offset < 0)
					tz_offset = -tz_offset;
				int hours = tz_offset  / 3600;
				int minutes = tz_offset / 60 % 60;
				ss << sign << hours;
				if(minutes!=0)
					ss << ':' << minutes;
			}
			ss << ";";
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

		struct stream::data {};
		stream::stream(std::ostream &s) : out_(&s)
		{
		}
		stream::~stream()
		{
		}
		void stream::log(message const &msg)
		{
			(*out_) << format_plain_text_message(msg) << std::endl;
		}
		
		struct file::data { booster::nowide::fstream stream; };
		file::file() :
			max_files_(0),
			max_size_(0),
			current_size_(0),
			opened_(false),
			append_(false),
			use_local_time_(true),
			tz_offset_(0),
			d(new file::data())
		{
			d->stream.imbue(std::locale::classic());
		}
		file::file(std::string const &file_name,int mf) :
			max_files_(0),
			max_size_(0),
			current_size_(0),
			opened_(false),
			append_(false),
			use_local_time_(true),
			tz_offset_(0),
			d(new file::data())
		{
			if(mf == app) {
				append();
			}
			else if(mf > 0) {
				max_files(mf);
			}
			d->stream.imbue(std::locale::classic());
			open(file_name);
		}
		file::~file()
		{
		}
		
		void file::max_files(unsigned m)
		{
			if(!opened_) max_files_=m;
		}

		void file::set_timezone(std::string const &name)
		{
			if(name.empty()) {
				use_local_time_=true;
			}
			else {
				tz_offset_ = booster::locale::util::parse_tz(name);
				use_local_time_=false;
			}
		}

		void file::open(std::string file_name)
		{
			if(max_files_ > 0)
				shift(file_name);
			
			if(append_)
				d->stream.open(file_name.c_str(),std::fstream::out | std::fstream::app);
			else
				d->stream.open(file_name.c_str(),std::fstream::out);

			if(!d->stream)
				throw booster::runtime_error("Failed to open file " + file_name);
		}
		std::string file::format_file(std::string const &base,int n)
		{
			std::ostringstream ss;
			ss.imbue(std::locale::classic());
			ss << base << "." << n;
			return ss.str();
		}
		void file::append() 
		{
			append_ = true;
		}
		void file::shift(std::string const &base)
		{
			booster::nowide::remove(format_file(base,max_files_).c_str());
			for(unsigned file = max_files_-1;file > 0 ; file --) {
				booster::nowide::rename(format_file(base,file).c_str(),format_file(base,file+1).c_str());
			}
			
			booster::nowide::rename(base.c_str(),format_file(base,1).c_str());
		}
		void file::log(message const &msg)
		{
			if(use_local_time_)
				d->stream << format_plain_text_message(msg) << std::endl;
			else
				d->stream << format_plain_text_message_tz(msg,tz_offset_) << std::endl;
		}
		#ifdef BOOSTER_POSIX
		struct syslog::data {
			std::string id;
			bool log_was_opened;
			data() : log_was_opened(false) {}
		};
		syslog::syslog(int opts,int facility) : 
			d(new data())
		{
			d->log_was_opened = true;
			openlog(NULL,opts,facility);
		}
		syslog::syslog(std::string const &id,int opts,int facility) : 
			d(new data())
		{
			d->id = id;
			d->log_was_opened = true;
			openlog(d->id.c_str(),opts,facility);
		}
		syslog::syslog() : d(new data())
		{
		}
		syslog::~syslog()
		{
			if(d.get() && d->log_was_opened)
				closelog();
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
