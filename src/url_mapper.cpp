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
#include <cppcms/url_mapper.h>
#include <cppcms/cppcms_error.h>
#include <map>

#include <stdlib.h>

namespace cppcms {
	struct url_mapper::data 
	{
		struct entry {
			std::vector<std::string> parts;
			std::vector<int> indexes;
			std::vector<std::string> keys;
		};

		typedef std::map<size_t,entry> by_size_type;
		typedef std::map<std::string,by_size_type> by_key_type;
		typedef std::map<std::string,std::string> helpers_type;

		by_key_type by_key;
		helpers_type helpers;
		std::string root;

		bool map(	std::string const key,
				std::vector<std::string> const &params,
				std::string &output) const
		{
			by_key_type::const_iterator kp = by_key.find(key);
			if(kp == by_key.end())
				return false;
			by_size_type::const_iterator sp = kp->second.find(params.size());
			if(sp == kp->second.end())
				return false;

			output.clear();
			output+=root;

			entry const &formatting = sp->second;

			for(size_t i=0;i<formatting.parts.size();i++) {
				output += formatting.parts[i];
				if( i < formatting.indexes.size() ) {
					if(formatting.indexes[i]==0) {
						std::string const &hkey = formatting.keys[i];
						std::map<std::string,std::string>::const_iterator p = helpers.find(hkey);
						if(p != helpers.end()) {
							output += p->second;
						}
					}
					else {
						output+=params.at(formatting.indexes[i] - 1);
					}
				}
			}

			return true;	

		}

	};

	void url_mapper::assign(std::string const &key,std::string const &url)
	{
		data::entry e;
		std::string::const_iterator prev = url.begin(), p = url.begin();

		int max_index = 0;

		while(p!=url.end()) {
			if(*p=='{') {
				e.parts.push_back(std::string(prev,p));
				prev = p;
				while(p!=url.end()) {
					if(*p=='}') {
						std::string const hkey(prev+1,p);
						prev = p+1;
						if(hkey.size()==0) {
							throw cppcms_error("cppcms::url_mapper: empty index between {}");
						}
						bool all_digits = true;
						for(unsigned i=0;all_digits && i<hkey.size();i++) {
							if(hkey[i] < '0' || '9' <hkey[i])
								all_digits = false;
						}
						if(!all_digits) {
							e.indexes.push_back(0);
							e.keys.push_back(hkey);
						}
						else {
							int index = atoi(hkey.c_str());
							if(index == 0)
								throw cppcms_error("cppcms::url_mapper: index 0 is invalid");
							max_index = std::max(index,max_index);
							e.indexes.push_back(index);
							e.keys.resize(e.keys.size()+1);
						}
						break;
					}
					else
						p++;
				}
				if(p==url.end())
					throw cppcms_error("cppcms::url_mapper: '{' in url without '}'");
				p++;
			}
			else if(*p=='}') {
				throw cppcms_error("cppcms::url_mapper: '}' in url without '{'");
			}
			else
				p++;
		}
		e.parts.push_back(std::string(prev,p));
		d->by_key[key][max_index] = e;
	}

	void url_mapper::set_value(std::string const &key,std::string const &value)
	{
		d->helpers[key]=value;
	}
	void url_mapper::clear_value(std::string const &key)
	{
		d->helpers.erase(key);
	}

	url_mapper::url_mapper() : d(new url_mapper::data())
	{
	}
	url_mapper::~url_mapper()
	{
	}

	std::string url_mapper::root()
	{
		return d->root;
	}

	void url_mapper::root(std::string const &r)
	{
		d->root = r;
	}

	std::string url_mapper::real_map(std::string const &key,std::vector<std::string> const &params)
	{
		std::string result;
		if(!d->map(key,params,result)) {
			throw cppcms_error("cppcms::url_mapper:invalid key `" + key + "' given");
		}
		return result;
	}

	void url_mapper::map(	std::ostream &out,	
				std::string const &key)
	{
		std::vector<std::string> params;
		out << real_map(key,params);
	}

	void url_mapper::map(	std::ostream &out,	
				std::string const &key,
				filters::streamable const &p1)
	{
		std::vector<std::string> params(1);
		params[0] = p1.get(out);
		out << real_map(key,params);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2)
	{
		std::vector<std::string> params(2);
		params[0] = p1.get(out);
		params[1] = p2.get(out);
		out << real_map(key,params);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3)
	{
		std::vector<std::string> params(3);
		params[0] = p1.get(out);
		params[1] = p2.get(out);
		params[2] = p3.get(out);
		out << real_map(key,params);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4)
	{
		std::vector<std::string> params(4);
		params[0] = p1.get(out);
		params[1] = p2.get(out);
		params[2] = p3.get(out);
		params[3] = p4.get(out);
		out << real_map(key,params);
	}

	
}
