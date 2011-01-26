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
#ifndef CPPCMS_URL_MAPPER_H
#define CPPCMS_URL_MAPPER_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <cppcms/filters.h>
#include <string>
#include <vector>


namespace cppcms {
	///
	/// \brief class for mapping URLs - oposite of dispatch
	///
	class CPPCMS_API url_mapper : public booster::noncopyable {
	public:
		url_mapper();
		~url_mapper();
		
		std::string root();
		void root(std::string const &r);

		void assign(std::string const &key,std::string const &url);

		void set_value(std::string const &key,std::string const &value);
		void clear_value(std::string const &key);
		
		void map(	std::ostream &out,
				std::string const &key);

		void map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1);

		void map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2);

		void map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3);

		void map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4);
	private:
		std::string real_map(std::string const &key,std::vector<std::string> const &params);

		struct data;
		booster::hold_ptr<data> d;
	};

};

#endif
