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
#ifndef CPPCMS_MOUNT_POINT_H
#define CPPCMS_MOUNT_POINT_H

#include <cppcms/defs.h>
#include <string>
#include <booster/perl_regex.h>
#include <booster/copy_ptr.h>

namespace cppcms {
	class CPPCMS_API mount_point {
	public:
		typedef enum {
			match_path_info,
			match_script_name
		} selection_type;

		booster::regex host() const;
		booster::regex script_name() const;
		booster::regex path_info() const;
		int group() const;
		selection_type selection() const;

		void host(booster::regex const &);
		void script_name(booster::regex const &);
		void path_info(booster::regex const &);
		void group(int);
		void selection(selection_type);

		std::pair<bool,std::string> match(std::string const &h,std::string const &s,std::string const &p) const;

		mount_point();
		~mount_point();
		mount_point(mount_point const &);
		mount_point const &operator=(mount_point const &);
	private:
		booster::regex host_;
		booster::regex script_name_;
		booster::regex path_info_;
		int group_;
		selection_type selection_;
		struct _data;
		booster::copy_ptr<_data> d;
	};

} // cppcms

#endif


