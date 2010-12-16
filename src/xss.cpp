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
#include <cppcms/xss.h>

namespace cppcms { namespace xss { 

	struct rules::data {
		typedef std::map<std::string,booster::regex> properties_type;
		struct tag {
			properties_type properties;
			tag_type type;
		};
		typedef std::map<std::string,tag> tags_type;
		tags_type tags;
		std::map<std::string,std::string> entities;
		html_type html;
	};

	booster::shared_ptr<rules::data> rules::impl() const
	{
		return impl_;
	}
	
	booster::shared_ptr<rules::data> rules::impl()
	{
		// COW semantics 
		if(impl_.use_count()==1)
			return impl_;

		pimpl tmp(new data(*impl_));
		impl_.reset();
		impl_ = tmp;

		return impl_;
	}

	bool rules::valid_tag(std::string const &tag,tag_type type) const
	{
		pimpl dp=impl();
		data const *d=dp.get();
		data::tags_type::const_iterator p = d->tags.find(tag);
		if(p==d->tags.end())
			return false;
		switch(type) {
		case opening_and_closing:
			switch(p->type) {
			case opening_and_closing:
			case any_tag:
				return true;
			default:
				return false;
			}
			break;
		case stand_alone:
			switch(p->type) {
			case stand_alone:
			case any_tag:
				return true;
			default:
				return false;
			}
			break;
		default:
			return false;
		};
	}
	bool rules

	

	


} } // xss
