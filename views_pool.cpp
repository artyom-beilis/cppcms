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
#include "views_pool.h"
#include "json.h"
#include "config.h"
#include "cppcms_error.h"
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/format.hpp>
#else // Internal Boost
#   include <cppcms_boost/format.hpp>
    namespace boost = cppcms_boost;
#endif

#include <booster/thread.h>

#ifdef CPPCMS_WIN32
#include <windows.h>
#include <process.h>
#else
#include <dlfcn.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>



namespace cppcms {
namespace impl {
#ifdef CPPCMS_WIN32
	class shared_object : public util::noncopyable {
	public:
		static std::string name(std::string file,std::string path);

		shared_object(std::string file_name,bool reloadable)
		{
			if(reloadable) {
				#ifdef CPPCMS_WIN_NATIVE
				int pid=_getpid();
				#else
				int pid=getpid();
				#endif
				file_name_ = (boost::format("%1%.tmp-%2%.dll",std::locale::classic()) % file_name % pid).str();
				if(!CopyFile(file_name.c_str(),file_name_.c_str(),1)) {
					throw cppcms_error("Failed to copy file "+file_name+" to "+file_name_);
				}
				remove_=true;
			}
			else {
				file_name_ = file_name;
				remove_=false;
			}
			handler_ = LoadLibrary(file_name_.c_str());
			if(!handler_) {
				if(remove_)
					DeleteFile(file_name_.c_str());
				throw cppcms_error("Failed to load library "+file_name);
			}
		}

		~shared_object()
		{
			FreeLibrary(handler_);
			if(remove_) {
				DeleteFile(file_name_.c_str());
			}
		}

		FARPROC symbol(std::string const &name) const
		{
			return GetProcAddress(handler_,name.c_str());
		}
	private:
		HMODULE handler_;
		std::string file_name_;
		bool remove_;
	};
#else
	class shared_object : public util::noncopyable {
	public:
		static std::string name(std::string file,std::string path);

		shared_object(std::string file_name,bool unused)
		{
			handler_ = dlopen(file_name.c_str(),RTLD_LAZY);
			if(!handler_) {
				throw cppcms_error("Failed to load library "+file_name);
			}
		}

		~shared_object()
		{
			dlclose(handler_);
		}

		void *symbol(std::string const &name) const
		{
			return dlsym(handler_,name.c_str());
		}
	private:
		void *handler_;
	};
#endif

	std::string shared_object::name(std::string file,std::string path)
	{
	#ifdef CPPCMS_LIBRARY_PREFIX
		return path+"/"+CPPCMS_LIBRARY_PREFIX+file+CPPCMS_LIBRARY_SUFFIX;
	#else
		return path+"/"+file+CPPCMS_LIBRARY_SUFFIX;
	#endif
	}


} // impl

#if !defined(HAVE_STAT) && defined(HAVE__STAT)
#define stat _stat
#endif

struct views_pool::skin {
public:
	skin()
	{
	}

	skin(std::string const &name,std::vector<std::string> const &search_path,bool reloadable) :
		reloadable_(true),
		skin_name_(name)
	{
		for(unsigned i=0;i<search_path.size();i++) {
			struct stat st;
			std::string file_name = impl::shared_object::name(name,search_path[i]);
			if(::stat(file_name.c_str(),&st) < 0)
				continue;
			file_name_ = file_name;
			time_stamp_ = st.st_mtime;

			shared_object_.reset(new impl::shared_object(file_name,reloadable));
			return;
		}
		throw cppcms_error("Can't load skin " + name); 
	}
	
	void copy_mapping(skin const &other)
	{
		mapping_ = other.mapping_;
	}


	skin(std::string skin_name,std::map<std::string,view_factory_type> const &views) :
		reloadable_(false),
		skin_name_(skin_name)
	{
		mapping_=views;
	}

	bool is_updated() const
	{
		if(!reloadable_)
			return true;
		struct stat st;
		if(::stat(file_name_.c_str(),&st) < 0)
			return true;
		return time_stamp_ >= st.st_mtime;
	}

	void render(std::string template_name,std::ostream &output,base_content &content) const
	{
		mapping_type::const_iterator p = mapping_.find(template_name);
		if(p==mapping_.end())
			throw cppcms_error("Can't find template "+template_name + " in skin "+skin_name_);
		std::auto_ptr<base_view> a_view = p->second(output,&content);
		a_view->render();
	}

private:
	bool reloadable_;
	std::string skin_name_;
	std::string file_name_;
	time_t time_stamp_;
	mapping_type mapping_;
	boost::shared_ptr<impl::shared_object> shared_object_;	

};

struct views_pool::data {
	bool dynamic_reload;
	typedef std::map<std::string,skin> skins_type;
	skins_type skins;
	booster::shared_mutex lock_;
	std::string default_skin;
	std::vector<std::string> search_path;
};

views_pool::views_pool() :
	d(new data())
{
}

std::string views_pool::default_skin() const
{
	return d->default_skin;
}

views_pool::views_pool(json::value const &settings) :
	d(new data())
{
	d->skins=static_instance().d->skins;
	std::vector<std::string> paths=settings.get("views.paths",std::vector<std::string>());
	d->search_path=paths;
	std::vector<std::string> skins=settings.get("views.skins",std::vector<std::string>());
	d->dynamic_reload= settings.get("views.auto_reload",false);
	d->default_skin = settings.get<std::string>("views.default_skin","");
	if(d->default_skin.empty() && d->skins.size()==1)
		d->default_skin=d->skins.begin()->first;
	if(paths.empty() || skins.empty()) {
		return;
	}
	for(unsigned i=0;i<skins.size();i++) {
		std::string name=skins[i];
		if(d->skins.find(name)!=d->skins.end())
			throw cppcms_error("Two skins with same name provided:" + name);
		d->skins[name]=skin(name,paths,d->dynamic_reload);
	}
	if(d->default_skin.empty())
		d->default_skin=skins[0];
}

void views_pool::render(std::string skin_name,std::string template_name,std::ostream &out,base_content &content)
{
	if(d->dynamic_reload) {
		for(;;){
			{	// Check if update
				booster::shared_lock<booster::shared_mutex> lock(d->lock_);
				data::skins_type::const_iterator p=d->skins.find(skin_name);
				if(p==d->skins.end())
					throw cppcms_error("There is no such skin:" + skin_name);
				if(p->second.is_updated()) {
					p->second.render(template_name,out,content);
					return;
				}
			}
			{	// Reload
				booster::unique_lock<booster::shared_mutex> lock(d->lock_);
				data::skins_type::iterator p=d->skins.find(skin_name);
				if(p==d->skins.end())
					throw cppcms_error("There is no such skin:" + skin_name);
				if(!p->second.is_updated()) {
					d->skins.erase(p);
					skin new_skin = skin(skin_name,d->search_path,true);
					new_skin.copy_mapping(d->skins[skin_name]);
					d->skins[skin_name] = new_skin;
				}
			}
		}
	}
	else {	// No need to reload
		data::skins_type::const_iterator p=d->skins.find(skin_name);
		if(p==d->skins.end())
			throw cppcms_error("There is no such skin:" + skin_name);
		p->second.render(template_name,out,content);
	}
}

views_pool::~views_pool()
{
}

void views_pool::add_view(std::string name,mapping_type const &mapping)
{
	data::skins_type::iterator p=d->skins.find(name);
	if(p!=d->skins.end())
		throw cppcms_error("Skin " + name + "can't be loaded twice");
	d->skins[name]=skin(name,mapping);
}

void views_pool::remove_view(std::string name)
{
	d->skins.erase(name);
}


namespace { // Make sure that static views pool is loaded
	struct loader {
		loader()
		{
			views_pool::static_instance();
		}
	};
} // anon


views_pool &views_pool::static_instance()
{
	static views_pool pool;
	return pool;
}


} // cppcms
