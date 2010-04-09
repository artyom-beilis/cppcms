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
#ifndef CPPCMS_LOGGER_H
#define CPPCMS_LOGGER_H

#include "defs.h"
#include <iosfwd>
#include <memory>

#include "copy_ptr.h"
#include "hold_ptr.h"
#include "noncopyable.h"

namespace cppcms {
	class CPPCMS_API logger : public util::noncopyable {
	public:
		typedef enum {
			fatal	= 10,
			critical= 20,
			error	= 30,
			warning	= 40,
			message	= 50,
			info	= 60,
			debug	= 70,
			all	= 100
		} level_type;

		class CPPCMS_API ostream_proxy {
		public:	
			ostream_proxy();
			ostream_proxy(level_type level,char const *module,char const *file,int line,logger *log);
			~ostream_proxy();
			ostream_proxy(ostream_proxy &other);
			ostream_proxy &operator=(ostream_proxy &other);
			std::ostream &out();
		private:
			level_type level_;
			int line_;
			char const *file_;
			char const *module_;
			logger *log_;
			struct data;
			util::copy_ptr<data> d;
			std::auto_ptr<std::ostringstream> output_; 
		};
		

		void write_to_log(level_type l,char const *module,char const *file,int line,std::string const &message);

		bool level(level_type l,char const *module);
		ostream_proxy proxy(level_type l,char const *module,char const *file,int line);

		void default_level(level_type l);
		level_type default_level();
		
		void module_level(char const *module,level_type l);
		level_type module_level(char const *module);
		void reset_module_level(char const *module);
		
		typedef enum {
			overwrite = 0,
			append = 1
		} open_mode_type;
	
		void log_to_file(std::string const &file_name,open_mode_type mode = append);
		void log_to_stdout();
		void log_to_stderr();
		static logger &instance();
	private:
		static void init(std::auto_ptr<logger> &logger_ref);
		struct data;
		util::hold_ptr<data> d;
	};

	
	#define CPPCMS_LOG(_l,_m)								\
		::cppcms::logger::instance().level(::cppcms::logger::_l,_m) 				\
		&& ::cppcms::logger::instance().proxy(::cppcms::logger::_l,_m,__FILE__,__LINE__).out()

	#define CPPCMS_FATAL(_m)	CPPCMS_LOG(fatal,_m)
	#define CPPCMS_CRITICAL(_m)	CPPCMS_LOG(critical,_m)
	#define CPPCMS_ERROR(_m)	CPPCMS_LOG(error,_m)
	#define CPPCMS_WARNING(_m)	CPPCMS_LOG(warning,_m)
	#define CPPCMS_MESSAGE(_m)	CPPCMS_LOG(message,_m)
	#define CPPCMS_INFO(_m)		CPPCMS_LOG(info,_m)
	#define CPPCMS_DEBUG(_m)	CPPCMS_LOG(debug,_m)


} // CppCMS

#endif
