#define CPPCMS_SOURCE
#include "views_pool.h"
#include <boost/thread.hpp>
#include <boost/format.hpp>

#ifdef CPPCMS_WIN32
#include <windows.h>
#include <process.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

namespace cppcms {
namespace impl {
#ifdef CPPCMS_WIN32
	class shared_object : public util::noncopyable {
	public:
		static std::string name(std::string file,std::string path)
		{
			return path+"/"+file+".dll";
		}

		shared_object(std::string file_name,bool reloadable)
		{
			if(reloadable) {
				#ifdef CPPCMS_WIN_NATIVE
				int pid=_getpid();
				#else
				int pid=getpid()
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
				throw cppcms_error("Failed to load library "+file);
			}
		}

		~shared_object()
		{
			FreeLibrary(handler_);
			if(remove_) {
				DeleteFile(file_name_.c_str())
			}
		}

		void *symbol(std::string const &name) const
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
		static std::string name(std::string file,std::string path)
		{
			return path+"/"+file+CPPCMS_LIBRARY_SUFFIX;
		}

		shared_object(std::string file_name,bool unused)
		{
			handler_ = dlopen(file_name.c_str(),RTLD_LAZY);
			if(!handler_) {
				throw cppcms_error("Failed to load library "+file);
			}
		}

		~shared_object()
		{
			dlclose(handler_)
		}

		void *symbol(std::string const &name) const
		{
			return dlsym(handler_,name.c_str());
		}
	private:
		void *handler_;
	};
#endif
} // impl

#if !defined(HAVE_STAT) && defined(HAVE__STAT)
#define stat _stat
#endif

struct views_pool::skin {
public:

	skin(std::string const &name,std::vector<std::string> search_path,bool reloadable) :
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

			shared_object_.reset(new impl::shared_object(file_name,reloadable))
			typedef void (*loader_type)(mapping_type *);
			loader_type loader=reinterpret_cast<loader_type>(shared_object_->get_symbol("cppcms_"+name+"_get_skins"));
			if(!loader) {
				throw cppcms_error(path + " is not CppCMS loadable skin");
			}
			loader(mapping_);
			return;
		}
		throw cppcms_error("Can't load skin " + name); 
	}

	skin(std::string skin_name,std::map<std::string,view_factory_type> const &views) :
		reloadable_(false),
		name_(skin_name)
	{
		mapping_=views;
	}

	bool is_updated() const
	{
		if(!reloadable_)
			return true;
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
	mapping_type mapping_;
	boost::shared_ptr<impl::shared_object> shared_object_;	

};

struct views_pool::data {
	bool dyanmic_reload;
	boost::shared_mutex lock_;
};

views_pool::views_pool()
{
}

views_pool::views_pool(json::valuie const &settings)
{
	d->skins=instance().d->skins;
	std::vector<std::string> paths=settings.get("views.paths",std::vector<std::string>());
	std::vector<std::string> skins=settings.get("views.skins",std::vector<std::string>());
	d->dynamic_reload= settings.get("views.auto_reload",false);
	if(paths.empty() || skins.empty())
		return;
	for(unsigned i=0;i<skins.size();i++) {
		std::string name=skins[i];
		if(d->skins.find(name)!=d->skins.end())
			throw cppcms_error("Two skins with same name provided:" + name);
		d->skins[name]=skin(name,paths,d->dynamic_reload);
	}
	d->default_skin = settings.get<std::string>("views.default_skin","")
}

void views_pool::render(std::string skin,std::string template_name,std::ostream &out,base_content &content)
{
	if(d->dyanmic_reload) {
		for(;;){
			{	// Check if update
				boost::shared_lock<boost::shared_mutex> lock(d->lock_);
				data::skins_type::const_iterator p=d->skins.find(skin);
				if(p==d->skins.end())
					throw cppcms_error("There is no such skin:" + skin);
				if(p->second.is_updated()) {
					p->second.render(template_name,out,content);
					return;
				}
			}
			{	// Reload
				boost::unique_lock<boost::shared_mutex> lock(d->lock_);
				data::skins_type::iterator p=d->skins.find(skin);
				if(p==d->skins.end())
					throw cppcms_error("There is no such skin:" + skin);
				if(!p->second.is_updated()) {
					d->skins.erase(p);
					d->skins.insert(skins(skin,d->search_path,true));
				}
			}
		}
	}
	else {	// No need to reload
		data::skins_type::const_iterator p=d->skins.find(skin);
		if(p==d->skins.end())
			throw cppcms_error("There is no such skin:" + skin);
		d->second.render(template_name,out,content);
	}
}

views_pool::~views_pool()
{
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
