//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOGGER_H
#define BOOSTER_LOGGER_H

#include <booster/config.h>
#include <iosfwd>
#include <memory>
#include <booster/copy_ptr.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>


namespace booster {

template<typename T>
class shared_ptr;
template<typename T>
class weak_ptr;

namespace log {
	
	typedef enum {
		emergency	= 0,
		alert		= 10,
		critical	= 20,
		error		= 30,
		warning		= 40,
		notice		= 50,
		info		= 60,
		debug		= 70,
		all		= 100
	} level_type;

	class BOOSTER_API message {
	public:
		message(level_type l,char const *m,char const *name,int line);
		
		message();
		~message();
		message(message &);
		message &operator=(message &);

		level_type level() const;
		char const *module() const;
		char const *file_name() const;
		int file_line() const;
		std::string log_message() const;

		std::ostream &out();
	private:
		level_type level_;
		char const *module_;
		char const *file_name_;
		int file_line_;

		std::auto_ptr<std::ostringstream> message_;

		struct data;
		copy_ptr<data> d;
	};

	class BOOSTER_API sink : public noncopyable {
	public:
		virtual void log(message const &) = 0;
		virtual ~sink() {}
	};

	class BOOSTER_API logger : public noncopyable {
	public:
		static logger &instance();
		
		bool should_be_logged(level_type level,char const *module);
		
		void set_log_level(level_type level,char const *module);
		void reset_log_level(char const *module);
		void set_default_level(level_type level);

		void add_sink(shared_ptr<sink> const &s);
		void remove_sink(weak_ptr<sink> const &s);
		void remove_all_sinks();

		void log(message const &);

		static char const *level_to_string(level_type level);

	private:
		
		struct entry {
			char const *module;
			level_type level;
		};

		static const int max_entries_size_ = 1024;
		level_type default_level_;
		entry entries_[max_entries_size_];
		int entries_size_;
		
		struct data;
		hold_ptr<data> d;

		logger();
		~logger();
	};

	namespace sinks {
		
		BOOSTER_API std::string format_plain_text_message(message const &msg);

		class BOOSTER_API standard_error : public sink {
		public:
			standard_error();
			virtual void log(message const &);
			virtual ~standard_error();
		private:
			struct data;
			hold_ptr<data> d;
		};
		
		class BOOSTER_API file : public sink {
		public:
			file();
			virtual ~file();
			void open(std::string file_name);
			void max_files(unsigned limit);
			void append();
			
			virtual void log(message const &);
		private:

			void shift(std::string const &base);
			std::string format_file(std::string const &,int);

			hold_ptr<std::fstream> file_;
			unsigned max_files_;
			size_t max_size_;
			size_t current_size_;
			bool opened_;
			bool append_;
			
			struct data;
			hold_ptr<data> d;
		};

		#ifdef BOOSTER_POSIX
		class BOOSTER_API syslog : public sink {
		public:
			syslog();
			virtual void log(message const &);
			virtual ~syslog();
		private:
			struct data;
			hold_ptr<data> d;
		};
		#endif

	} // sinks


	#define BOOSTER_LOG(_l,_m) 								\
		::booster::log::logger::instance().should_be_logged(::booster::log::_l,_m)	\
		&& ::booster::log::message(::booster::log::_l,_m,__FILE__,__LINE__).out()			

	
	#define BOOSTER_EMERG(_m)	BOOSTER_LOG(emergency,_m)
	#define BOOSTER_ALERT(_m)	BOOSTER_LOG(alert,_m)
	#define BOOSTER_CRITICAL(_m)	BOOSTER_LOG(critical,_m)
	#define BOOSTER_ERROR(_m)	BOOSTER_LOG(error,_m)
	#define BOOSTER_WARNING(_m)	BOOSTER_LOG(warning,_m)
	#define BOOSTER_NOTICE(_m)	BOOSTER_LOG(notice,_m)
	#define BOOSTER_INFO(_m)	BOOSTER_LOG(info,_m)
	#define BOOSTER_DEBUG(_m)	BOOSTER_LOG(debug,_m)

} // log

} // booster

#endif
