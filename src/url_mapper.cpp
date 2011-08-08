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
#include <cppcms/application.h>
#include <cppcms/cppcms_error.h>
#include <map>

#include <stdlib.h>

#include <iostream>

namespace cppcms {

	template<typename Data,typename Entry>
	struct stream_it {
		Data const *self;
		Entry const &formatting;
		filters::streamable const * const *params;
		size_t params_no;
		std::map<string_key,std::string> const &data_helpers_default;
		std::map<string_key,std::string> const &data_helpers_override;
		void write(std::ostream &out) const
		{
			self->write(formatting,params,params_no,data_helpers_default,data_helpers_override,out);
		}
	};

	template<typename Data,typename Entry>
	std::ostream &operator<<(std::ostream &out,stream_it<Data,Entry> const &wr)
	{
		wr.write(out);
		return out;
	}


	struct url_mapper::data 
	{

		data() : parent(0),this_application(0) {}
		std::string this_name;
		application *parent;
		application *this_application;

		struct entry {
			std::vector<std::string> parts;
			std::vector<int> indexes;
			std::vector<std::string> keys;
			application *child;
			entry() : child(0) {}
		};
		
		template<typename Data,typename Entry>
		friend struct stream_it;

		typedef std::map<size_t,entry> by_size_type;
		typedef std::map<string_key,by_size_type> by_key_type;
		typedef std::map<string_key,std::string> helpers_type;

		by_key_type by_key;
		helpers_type helpers;
		std::string root;

		entry const &get_entry(string_key const &key,size_t params_no,string_key const &full_url) const
		{
			by_key_type::const_iterator kp = by_key.find(key);
			if(kp == by_key.end())
				throw cppcms_error("url_mapper: key `" + key.str() + "' not found for "
						"url `" + full_url.str() + "'");
			by_size_type::const_iterator sp = kp->second.find(params_no);
			if(sp == kp->second.end())
				throw cppcms_error("url_mapper: invalid number of parameters for " + key.str() +
						"in url `" + full_url.str() + "'");
			return sp->second;
		}
		
		url_mapper *is_app(string_key const &key) const
		{
			by_key_type::const_iterator kp = by_key.find(key);
			if(kp == by_key.end())
				return 0;
			by_size_type::const_iterator sp = kp->second.begin();
			if(sp==kp->second.end())
				return 0;
			if(sp->second.child) 
				return &sp->second.child->mapper();
			return 0;
		}

		url_mapper &child(string_key const &name,string_key const &full_url)
		{
			entry const &fmt = get_entry(name,1,full_url);
			if(!fmt.child) {
				throw cppcms_error("url_mapper: the key " + name.str() + " is not child application key"
						" in url `" + full_url.str() + "'");
			}
			return fmt.child->mapper();
		}

		struct map_stream {
			filters::streamable const * const *params;
			size_t params_no;
			std::map<string_key,std::string> const &data_helpers_default;
			std::map<string_key,std::string> const &data_helpers_override;
		};

		void write(	entry const &formatting,
				filters::streamable const * const *params,
				size_t params_no,
				std::map<string_key,std::string> const &data_helpers_default,
				std::map<string_key,std::string> const &data_helpers_override,
				std::ostream &out) const
		{
			for(size_t i=0;i<formatting.parts.size();i++) {
				out << formatting.parts[i];
				if( i < formatting.indexes.size() ) {
					if(formatting.indexes[i]==0) {
						string_key hkey  = string_key::unowned(formatting.keys[i]);
						std::map<string_key,std::string>::const_iterator p;
						p = data_helpers_override.find(string_key::unowned(hkey));
						if(p != data_helpers_override.end()) {
							out << p->second;
						}
						else {
							p = data_helpers_default.find(hkey);
							if(p!=data_helpers_default.end())
								out << p->second;
						}
					}
					else {
						size_t index = formatting.indexes[i] - 1;
						if(index >= params_no) {
							throw cppcms_error("url_mapper: Index of parameter out of range");
						}
						(*params[index])(out);
					}
				}
			}
		}

		void map(	string_key const &key,
				string_key const &full_url,
				filters::streamable const * const *params,
				size_t params_no,
				std::map<string_key,std::string> const &data_helpers_default,
				std::map<string_key,std::string> const &data_helpers_override,
				std::ostream &out) const
		{
			entry const &formatting = get_entry(key,params_no,full_url);

			if(parent) {
				stream_it<data,entry> url = 
				{ 
					this,
					formatting,
					params,
					params_no,
					data_helpers_default,
					data_helpers_override
				};

				filters::streamable stream_url(url);
				filters::streamable const *par_ptr=&stream_url;
				parent->mapper().d->map(this_name,full_url,&par_ptr,1,	data_helpers_default,
											data_helpers_override,out);
			}
			else {
				out << root;
				write(formatting,params,params_no,data_helpers_default,data_helpers_override,out);
			}

		}


	};

	void url_mapper::assign(std::string const &key,std::string const &url)
	{
		if(	key.empty() 
			|| key.find('/') != std::string::npos 
			|| key.find(';') != std::string::npos 
			|| key.find(',') != std::string::npos 
			|| key ==".." 
			|| key == "." )
		{
			throw cppcms_error("cppcms::url_mapper: key may not be '' , '.' or '..' and must not include '/' in it");
		}
		real_assign(key,url,0);
	}
	void url_mapper::assign(std::string const &url)
	{
		real_assign(std::string(),url,0);
	}
	void url_mapper::real_assign(std::string const &key,std::string const &url,application *child)
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
		if(child && max_index!=1) {
			throw cppcms_error("cppcms::url_mapper the application mapping should use only 1 parameter");
		}
		e.parts.push_back(std::string(prev,p));

		e.child = child;

		if(child) {
			if(d->by_key.find(key)!=d->by_key.end()) 
				throw cppcms_error(	"cppcms::url_mapper: mounted application key `" + key +
							"' can't be shared with ordinary url key");
		}
		else {
			data::by_key_type::iterator p = d->by_key.find(key);
			if(p!=d->by_key.end()) {
				data::by_size_type::const_iterator p2 = p->second.find(1);
				if(p2!=p->second.end() && p2->second.child) 
				{
					throw cppcms_error(	"cppcms::url_mapper: ordinary url key `"+key+
								"can't be shared with mounted application key");
				}
			}
		}
		d->by_key[key][max_index] = e;
	}

	void url_mapper::set_value(std::string const &key,std::string const &value)
	{
		root_mapper()->d->helpers[key]=value;
	}
	void url_mapper::clear_value(std::string const &key)
	{
		root_mapper()->d->helpers.erase(key);
	}

	url_mapper &url_mapper::child(std::string const &name)
	{
		return d->child(name,name);
	}
	
	url_mapper *url_mapper::root_mapper()
	{
		if(d->parent)
			return &d->parent->root()->mapper();
		else
			return this;
	}

	void url_mapper::mount(std::string const &name,std::string const &url,application &app)
	{
		app.mapper().d->parent = d->this_application;
		app.mapper().d->this_name = name;
		real_assign(name,url,&app);
		// Copy all values to root most one
		std::map<string_key,std::string> &values = app.mapper().d->helpers;
		std::map<string_key,std::string>::const_iterator p;
		for(p=values.begin();p!=values.end();++p) {
			set_value(p->first,p->second);
		}
		values.clear();
	}

	url_mapper::url_mapper(cppcms::application *my_app) : d(new url_mapper::data())
	{
		d->this_application = my_app;
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


	url_mapper &url_mapper::get_mapper_for_key(string_key const &key,string_key &real_key,std::vector<string_key> &keywords)
	{
		url_mapper *mapper = this;
		size_t pos = 0;
		if(key.empty()) {
			real_key.clear();
			return *mapper;
		}
		if(key[0]=='/') {
			mapper = &mapper->topmost();
			pos = 1;
		}
		for(;;) {
			size_t end = key.find('/',pos);
			if(end == std::string::npos) {
				size_t separator = key.find(';',pos);
				if(separator == std::string::npos) {
					real_key = key.unowned_substr(pos);
				}
				else {
					keywords.reserve(6);
					real_key = key.unowned_substr(pos,separator - pos);
					pos = separator + 1;
					for(;;){
						end = key.find(',',pos);
						size_t size = end - pos;
						keywords.push_back(key.unowned_substr(pos,size));
						if(end == std::string::npos)
							break;
						pos = end + 1;
					}
					
				}
				if(real_key == ".")
					real_key.clear();
				else if(real_key == "..") {
					mapper = &mapper->parent();
					real_key.clear();
				}
					
				url_mapper *tmp = mapper->d->is_app(real_key);
				if(tmp) { 
					// empty special key
					real_key.clear();
					return *tmp;
				}
				return *mapper;
			}
			size_t chunk_size = end - pos;
			string_key subapp = key.unowned_substr(pos,chunk_size);
			if(subapp == ".")
				; // Just continue where we are
			else if(subapp == "..") {
				mapper = &mapper->parent();
			}
			else {
				mapper = &mapper->d->child(subapp,key);
			}
			pos = end + 1;
		}
	}


	void url_mapper::real_map(	char const *ckey,
					filters::streamable const * const *params,
					size_t params_no,
					std::ostream &output)
	{
		string_key key=string_key::unowned(ckey);
		string_key real_key;
		std::vector<string_key> direct;
		url_mapper &mp = get_mapper_for_key(key,real_key,direct);
		if(params_no < direct.size())
			throw cppcms_error("url_mapper: number of keywords is larger then number of actual parameters");
		std::map<string_key,std::string> mappings;
		if(direct.size() == 0) {
			mp.d->map(real_key,key,params,params_no,topmost().d->helpers,mappings,output);
		}
		else {
			size_t direct_size = direct.size();
			filters::streamable const * const *key_params = params;
			params += direct_size;
			params_no -= direct_size;
			for(unsigned i=0;i<direct.size();i++) {
				std::ostringstream ss;
				ss.copyfmt(output);
				(*key_params[i])(ss);
				mappings[direct[i]]=ss.str();
			}
			mp.d->map(real_key,key,params,params_no,topmost().d->helpers,mappings,output);
		}
	}
	
	void url_mapper::map(	std::ostream &out,	
				std::string const &key)
	{
		map(out,key.c_str());
	}

	void url_mapper::map(	std::ostream &out,	
				std::string const &key,
				filters::streamable const &p1)
	{
		map(out,key.c_str(),p1);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2)
	{
		map(out,key.c_str(),p1,p2);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3)
	{
		map(out,key.c_str(),p1,p2,p3);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4)
	{
		map(out,key.c_str(),p1,p2,p3,p4);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5)
	{
		map(out,key.c_str(),p1,p2,p3,p4,p5);
	}

	void url_mapper::map(	std::ostream &out,
				std::string const &key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5,
				filters::streamable const &p6)
	{
		map(out,key.c_str(),p1,p2,p3,p4,p5,p6);
	}

	void url_mapper::map(	std::ostream &out,	
				char const *key)
	{
		real_map(key,0,0,out);
	}

	void url_mapper::map(	std::ostream &out,	
				char const *key,
				filters::streamable const &p1)
	{
		filters::streamable const *params[1]= { &p1 };
		real_map(key,params,1,out);
	}

	void url_mapper::map(	std::ostream &out,
				char const *key,
				filters::streamable const &p1,
				filters::streamable const &p2)
	{
		filters::streamable const *params[2] = { &p1,&p2 };
		real_map(key,params,2,out);
	}

	void url_mapper::map(	std::ostream &out,
				char const *key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3)
	{
		filters::streamable const *params[3] = { &p1,&p2,&p3 };
		real_map(key,params,3,out);
	}

	void url_mapper::map(	std::ostream &out,
				char const *key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4)
	{
		filters::streamable const *params[4] = { &p1,&p2,&p3,&p4 };
		real_map(key,params,4,out);
	}

	void url_mapper::map(	std::ostream &out,
				char const *key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5)
	{
		filters::streamable const *params[5] = { &p1,&p2,&p3,&p4,&p5 };
		real_map(key,params,5,out);
	}

	void url_mapper::map(	std::ostream &out,
				char const *key,
				filters::streamable const &p1,
				filters::streamable const &p2,
				filters::streamable const &p3,
				filters::streamable const &p4,
				filters::streamable const &p5,
				filters::streamable const &p6)
	{
		filters::streamable const *params[6] = { &p1,&p2,&p3,&p4,&p5,&p6 };
		real_map(key,params,6,out);
	}

	url_mapper &url_mapper::parent()
	{
		if(d->parent) {
			return d->parent->mapper();
		}
		throw cppcms_error("url_mapper: no parent found");
	}
	
	url_mapper &url_mapper::topmost()
	{
		application *app = d->this_application;
		if(!app)
			return *this;
		while(app->mapper().d->parent)
			app = app->mapper().d->parent;
		return app->mapper();
	}

	
}
