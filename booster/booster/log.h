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
#include <string>
#include <booster/copy_ptr.h>
#include <booster/hold_ptr.h>
#include <booster/noncopyable.h>

/*! \file */ 


namespace booster {

template<typename T>
class shared_ptr;
template<typename T>
class weak_ptr;

///
/// \brief This namespace includes Booster.Log related classes
///

namespace log {
	
	///
	/// This enum defined severity level of logged messages
	/// 
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

	
	///
	/// \brief This class represents a single message that should be written to log
	///
	/// Note, it is some generalization of data that allows you to format any generic
	/// data, it generally used by logging sinks rather then by end users
	///
	class BOOSTER_API message {
	public:
		///
		/// Create message with severity level \a l, sent from module \a m in file
		/// \a name in line \a line
		///
		message(level_type l,char const *m,char const *name,int line);
	
		///
		/// Default constructable
		///	
		message();

		~message();
		/// Copy message
		message(message &);
		/// Assign the  message
		message &operator=(message &);

		///
		/// Get the severity level for the message
		///
		level_type level() const;
		///
		/// Get the module name
		///
		char const *module() const;
		///
		/// Get the file name
		///
		char const *file_name() const;
		///
		/// Get the number of line in the file
		///
		int file_line() const;
		///
		/// Get the actual message that should be written
		///
		std::string log_message() const;

		///
		/// Get the output stream that consumes the message
		///
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

	///
	/// \brief This is the abstract interface to general sink - the consumer
	/// of the logged messages
	///
	class BOOSTER_API sink : public noncopyable {
	public:
		///
		/// Send the message \a m to log
		///
		/// Note: it does not have to be thread safe - in terms that only
		/// one message is sent each time and it is promised that are 
		/// not sent in parallel
		///
		virtual void log(message const &m) = 0;
		virtual ~sink() {}
	};

	///
	/// \brief This is the central class that manages all logging operations.
	///
	/// This is singleton, you access it only via instance() static member function.
	///
	/// This class is thread safe
	///
	class BOOSTER_API logger : public noncopyable {
	public:
		///
		/// Get the instance of the logger
		///
		static logger &instance();
		
		///
		/// Test if the message with severity level \a level of module \a module should
		/// be logged. This is generally called before preparing a message in order
		/// to prevent logging messages that shouldn't be logged
		///
		bool should_be_logged(level_type level,char const *module);
		
		///
		/// Set specific severity level \a level for a module \a module. It would
		/// be logged with different logging level then others.
		///
		/// Note: \a module string should remain valid all the time this log level
		/// is used. It is better to use static strings for these purposes
		///
		void set_log_level(level_type level,char const *module);

		///
		/// Reset \a module's logging severity to default
		///
		void reset_log_level(char const *module);
		///
		/// Set default logging level severity
		///
		void set_default_level(level_type level);

		///
		/// Add new logging sink - the target that receives messages.
		/// It is defined by it's pointer.
		///
		/// If you plan to remove it, keep the pointer on such logger.
		///
		void add_sink(shared_ptr<sink> const &s);

		///
		/// Remove a logging sink using its pointer. Note it is enough to keep
		/// weak pointer on it allowing the logger to destroy the sink.
		///
		void remove_sink(weak_ptr<sink> const &s);

		///
		/// Remove all logging sinks from the logger
		///
		void remove_all_sinks();

		///
		/// Send a message to the log, note, the message is sent regardless of its
		/// severity, use should_be_logged before you create the message
		///
		void log(message const &);

		///
		/// Get convert the severity level to string
		///
		static char const *level_to_string(level_type level);
		///
		/// Convert string to severity level, if the string is invalid, std::invalid_argument is thrown
		///
		static level_type string_to_level(std::string const &);

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

	///
	/// \brief This namespace includes various output devices (sinks) for logger.
	///
	/// It includes several standard devices that can be used in most applications
	///
	namespace sinks {
		
		///
		/// Convert a logging message to a string with all the data that can be
		/// written to file, the displayed time is local time
		///
		BOOSTER_API std::string format_plain_text_message(message const &msg);

		///
		/// Convert a logging message to a string with all the data that can be
		/// written to file, the displayed time is GMT+\a timezone_offset.
		/// timezone_offset should be represented in seconds, for example
		/// for timezone GMT+2:00 it should be 7200
		///
		BOOSTER_API std::string format_plain_text_message_tz(message const &msg,int timezone_offset = 0);

		///
		/// \brief stderr based sink - sends messages to standard error output
		///
		class BOOSTER_API standard_error : public sink {
		public:
			standard_error();
			virtual void log(message const &);
			virtual ~standard_error();
		private:
			struct data;
			hold_ptr<data> d;
		};
		
		///
		/// \brief log file based sink - sends messages to log file
		///
		class BOOSTER_API file : public sink {
		public:
			///
			/// Creates new object but does not open a file
			///
			file();
			virtual ~file();

			///
			/// Open log file
			///
			void open(std::string file_name);
			///
			/// Enable file rotation and set the maximal number of files that should be kept.
			/// 
			/// Each time the log opened, the old files are renamed, if there are more files 
			/// then \a limit, the oldest is removed
			///
			void max_files(unsigned limit);
			///
			/// Append to output file rather then create new one.
			///
			void append();
			
			///
			/// Set the time-zone name that should be used in the message.
			///
			/// It should have a format GMT+XX:YY like "GMT+2:00" or "GMT-3".
			/// "GMT" can be used as well
			///
			/// If name is empty local time is used which is the default
			///
			void set_timezone(std::string const &name);

			/// Send message to the log
			virtual void log(message const &);
		private:

			void shift(std::string const &base);
			std::string format_file(std::string const &,int);

			unsigned max_files_;
			size_t max_size_;
			size_t current_size_;
			bool opened_;
			bool append_;
			bool use_local_time_;
			int tz_offset_;
			
			struct data;
			hold_ptr<data> d;
		};

		#ifdef BOOSTER_POSIX
		///
		/// \brief POSIX syslog sink
		///
		/// Available only on POSIX platforms
		///
		class BOOSTER_API syslog : public sink {
		public:
			///
			/// Create a new logger. Note: it does not call openlog, if you want
			/// to provide non-default parameters, you need to call it on your own
			///
			syslog();
			///
			/// Send the message to the log
			///
			virtual void log(message const &);
			virtual ~syslog();
		private:
			struct data;
			hold_ptr<data> d;
		};
		#endif

	} // sinks


	///
	/// \def BOOSTER_LOG(level,module)
	///
	/// \brief Log a message with severity \a level of the \a module to the log device if applicable.
	///
	/// Notes:
	///
	/// - it first checks of the message should be logged at all, and if not, all the rest is not called
	/// - the message is logged in the destruction of the output stream object
	/// - the parameter \a level should be given as is "warning" or "debug" the namespace flags would
	///   be added withing macro
	/// - It may be more convenient to use BOOSTER_WARNING or BOOSTER_DEBUG macros
	///
	/// Example:
	///
	/// \code
	///   BOOSTER_LOG(warning,"file_system") << "Failed to open file name " << name;
	///   BOOSTER_CRITICAL("core") << "Failed to allocate memory";
	/// \endcode   
	///
	#define BOOSTER_LOG(level,module) 								\
		::booster::log::logger::instance().should_be_logged(::booster::log::level,module)	\
		&& ::booster::log::message(::booster::log::level,module,__FILE__,__LINE__).out()			

	
	/// Same as BOOSTER_LOG(emergency,m)
	#define BOOSTER_EMERG(m)	BOOSTER_LOG(emergency,m)
	/// Same as BOOSTER_LOG(alert,m)
	#define BOOSTER_ALERT(m)	BOOSTER_LOG(alert,m)
	/// Same as BOOSTER_LOG(critical,m)
	#define BOOSTER_CRITICAL(m)	BOOSTER_LOG(critical,m)
	/// Same as BOOSTER_LOG(error,m)
	#define BOOSTER_ERROR(m)	BOOSTER_LOG(error,m)
	/// Same as BOOSTER_LOG(warning,m)
	#define BOOSTER_WARNING(m)	BOOSTER_LOG(warning,m)
	/// Same as BOOSTER_LOG(notice,m)
	#define BOOSTER_NOTICE(m)	BOOSTER_LOG(notice,m)
	/// Same as BOOSTER_LOG(info,m)
	#define BOOSTER_INFO(m)	BOOSTER_LOG(info,m)
	/// Same as BOOSTER_LOG(debug,m)
	#define BOOSTER_DEBUG(m)	BOOSTER_LOG(debug,m)

} // log

} // booster

#endif
