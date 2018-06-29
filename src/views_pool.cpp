///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include <cppcms/views_pool.h>
#include <cppcms/json.h>
#include <cppcms/config.h>
#include <cppcms/cppcms_error.h>
#include <booster/locale/format.h>
#include <booster/shared_ptr.h>
#include <booster/thread.h>
#include <booster/system_error.h>

#include <booster/log.h>

#ifdef CPPCMS_WIN_NATIVE
#include <booster/nowide/convert.h>
#include <windows.h>
#include <process.h>
#else
#include <dlfcn.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <utility>

namespace cppcms {
namespace views {
// class generator
struct generator::data {};
generator::generator()
{
}
generator::~generator()
{
}

void generator::name(std::string const &n)
{
	name_ = n;
}
std::string generator::name() const
{
	return name_;
}

void generator::add_factory(std::string const &n,generator::view_factory_type *factory)
{
	views_[n]=factory;
}

std::unique_ptr<base_view> generator::create(	std::string const &view_name,
						std::ostream &output,
						base_content *content) const
{
	std::unique_ptr<base_view> result;
	views_type::const_iterator p = views_.find(view_name);
	if(p!=views_.end())
		result = p->second(output,content);
	return result;
}

std::vector<std::string> generator::enumerate() const
{
	std::vector<std::string> all;
	all.reserve(views_.size());
	for(views_type::const_iterator p=views_.begin(),e=views_.end();p!=e;++p) {
		all.push_back(p->first);
	}
	return all;
}

// class pool
struct pool::data {
	booster::recursive_shared_mutex lock;
	typedef std::map<std::string, generator const *> skin_generators_type;
	typedef std::map<std::string,skin_generators_type> generators_type;
	generators_type generators;
};

void pool::add(generator const &g)
{
	generator const *ptr = &g;
	std::string name = ptr->name();

	booster::unique_lock<booster::recursive_shared_mutex> guard(d->lock);
	data::generators_type::iterator s = d->generators.find(name);

	if (s == d->generators.end()) {
		d->generators[name] = data::skin_generators_type();
		s = d->generators.find(name);
	}

	std::vector<std::string> views = ptr->enumerate();

	for(std::vector<std::string>::iterator v=views.begin(), e=views.end();v!=e;++v) {
		data::skin_generators_type& skin = s->second;
		skin[*v] = ptr;
	}
}

void pool::remove(generator const &g)
{
	generator const *ptr = &g;
	std::string name = ptr->name();

	booster::unique_lock<booster::recursive_shared_mutex> guard(d->lock);

	data::generators_type::iterator s = d->generators.find(name);

	if (s == d->generators.end())
		return;

	data::skin_generators_type& skin = s->second;

	std::vector<std::string> views = ptr->enumerate();

	for(std::vector<std::string>::iterator v=views.begin(), e=views.end();v!=e;++v) {
		if (skin[*v] == ptr)
			skin.erase(*v);
	}

	if (skin.empty())
		d->generators.erase(name);
}

base_view *pool::create_view(std::string const &skin,std::string const &template_name,std::ostream &out,base_content &content)
{
	data::generators_type::iterator s=d->generators.find(skin);
	if(s==d->generators.end()) 
		throw cppcms_error("cppcms::views::pool: no such skin:" + skin);

	data::skin_generators_type const& reg_skin = s->second;

	data::skin_generators_type::const_iterator t=reg_skin.find(template_name);
	if(t==reg_skin.end())
		throw cppcms_error("cppcms::view::pool: no suck view:" + template_name + " is registered for skin: " + skin);

	std::unique_ptr<base_view> v;
	v = t->second->create(template_name,out,&content);
	if(!v.get())
		throw cppcms_error("cppcms::views::pool: no such view " + template_name + " in the skin " + skin);
	return v.release();
}

void pool::lock()
{
	d->lock.shared_lock();
}
void pool::unlock()
{
	d->lock.unlock();
}

struct view_lock::_data {};
view_lock::view_lock(std::string const &skin,std::string const &template_name,std::ostream &out,base_content &content) 
{
	pool &p = pool::instance();
	p.lock();
	try {
		view_.reset(p.create_view(skin,template_name,out,content));
	}
	catch(...) {
		p.unlock();
		throw;
	}
}
view_lock::~view_lock()
{
	view_.reset();
	pool::instance().unlock();
}

base_view &view_lock::view()
{
	return *view_;
}

void pool::render(std::string const &skin,std::string const &template_name,std::ostream &out,base_content &content)
{
	booster::shared_lock<booster::recursive_shared_mutex> guard(d->lock);
	{
		booster::hold_ptr<base_view> v(create_view(skin,template_name,out,content));
		v->render();
	}
}

std::vector<std::string> pool::enumerate()
{
	booster::shared_lock<booster::recursive_shared_mutex> guard(d->lock);
	std::vector<std::string> all;
	all.reserve(d->generators.size());
	for(data::generators_type::iterator p=d->generators.begin(),e=d->generators.end();p!=e;++p) {
		all.push_back(p->first);
	}
	return all;
}

pool::pool() : d(new data) {}
pool::~pool() {}

pool &pool::instance()
{
	static pool instance_;
	return instance_;
}

namespace {
	struct initializer { initializer() { pool::instance(); } } initializer_instance;
}

namespace impl {
#ifdef CPPCMS_WIN_NATIVE
	class shared_object : public booster::noncopyable {
	public:
		static std::string name(std::string file,std::string path);

		shared_object(std::string u8file_name,bool reloadable)
		{
			std::wstring file_name = booster::nowide::convert(u8file_name);
			if(reloadable) {
				int pid=_getpid();
				file_name_ = (booster::locale::wformat(L"{1}.tmp-{2}.dll") % file_name % pid).str(std::locale::classic());
				if(!CopyFileW(file_name.c_str(),file_name_.c_str(),1)) {
					booster::system::error_code e(GetLastError(),booster::system::windows_category);
					throw booster::system::system_error(e,"Failed to copy file "+u8file_name+" to "
										+booster::nowide::convert(file_name_));
				}
				remove_=true;
			}
			else {
				file_name_ = file_name;
				remove_=false;
			}
			handler_ = LoadLibraryW(file_name_.c_str());
			if(!handler_) {
				if(remove_)
					DeleteFileW(file_name_.c_str());
				booster::system::error_code e(GetLastError(),booster::system::windows_category);
				throw booster::system::system_error(e,"Failed to load library "+u8file_name);
			}
		}

		~shared_object()
		{
			FreeLibrary(handler_);
			if(remove_) {
				DeleteFileW(file_name_.c_str());
			}
		}

		FARPROC symbol(std::string const &name) const
		{
			return GetProcAddress(handler_,name.c_str());
		}
	private:
		HMODULE handler_;
		std::wstring file_name_;
		bool remove_;
	};
#else
	class shared_object : public booster::noncopyable {
	public:
		static std::string name(std::string file,std::string path);

		shared_object(std::string file_name,bool /*unused*/)
		{
			handler_ = dlopen(file_name.c_str(),RTLD_LAZY | RTLD_GLOBAL);
			if(!handler_) {
				booster::system::error_code e(errno,booster::system::system_category);
				throw booster::system::system_error(e,"Failed to load library "+file_name);
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


	time_t get_mtime(std::string const &file_name)
	{
		#ifdef CPPCMS_WIN_NATIVE
		struct _stat st;
		if(_wstat(booster::nowide::convert(file_name).c_str(),&st) < 0)
			return 0;
		#else
		struct stat st;
		if(stat(file_name.c_str(),&st) < 0)
			return 0;
		#endif
		return st.st_mtime;
	}

	struct skin {
		std::string file_name;
		booster::shared_ptr<impl::shared_object> so;
		time_t mtime;
	};

	std::string default_library_pattern()
	{
		#ifdef CPPCMS_LIBRARY_PREFIX
		return CPPCMS_LIBRARY_PREFIX "{1}" CPPCMS_LIBRARY_SUFFIX;
		#else
		return "{1}" CPPCMS_LIBRARY_SUFFIX;
		#endif
	}

} // impl


struct manager::data {
	bool auto_reload;
	std::string default_skin;
	std::vector<impl::skin> skins;
	booster::recursive_shared_mutex lock;
	data() : auto_reload(false) 
	{
	}
};


manager::manager(json::value const &settings) :
	d(new data())
{
	// configure all
	d->auto_reload = settings.get("views.auto_reload",false);
	std::string pattern = settings.get("views.shared_object_pattern",impl::default_library_pattern());
	std::vector<std::string> paths=settings.get("views.paths",std::vector<std::string>());
	std::vector<std::string> skins=settings.get("views.skins",std::vector<std::string>());
	if(!skins.empty() && paths.empty()) {
		throw cppcms_error("When views.skins provided at least one search path should be given in views.paths");
	}
	d->default_skin = settings.get<std::string>("views.default_skin","");
	if(d->default_skin.empty()) {
		if(!skins.empty())
			d->default_skin = skins.front();
		else  {
			std::vector<std::string> static_skins = pool::instance().enumerate();
			if(static_skins.size()==1)
				d->default_skin = static_skins[0];
		}
	}

	// configuration done  now load all
	for(unsigned i=0;i<skins.size();i++) {
		std::string name=skins[i];
		unsigned j;
		for(j=0;j<paths.size();j++) {
			impl::skin new_skin;
			new_skin.file_name = paths[j] + "/" + (booster::locale::format(pattern) % name).str();
			new_skin.mtime = impl::get_mtime(new_skin.file_name);
			if(new_skin.mtime!=0) {
				new_skin.so.reset(new impl::shared_object(new_skin.file_name,d->auto_reload));
				d->skins.push_back(new_skin);
				break;
			}
		}
		if(j == paths.size()) {
			throw cppcms_error("Failed to load skin:" + name+ ", no shared object/dll found");
		}
	}
}

manager::~manager()
{
}

std::string manager::default_skin()
{
	return d->default_skin;
}

void manager::render(std::string const &skin_name,std::string const &template_name,std::ostream &out,base_content &content)
{
	if(skin_name.empty() && d->default_skin.empty()) {
		throw cppcms_error("No default skin was detected, please define one in views.default_skin");
	}
	if(d->auto_reload) {
		{	// Check if update
			bool reload_required = false;
			booster::shared_lock<booster::recursive_shared_mutex> guard(d->lock);
			for(size_t i=0;i<d->skins.size();i++) {
				time_t mtime = impl::get_mtime(d->skins[i].file_name);
				if(mtime != d->skins[i].mtime) {
					reload_required = true;
					break;
				}
			}
			if(!reload_required) {
				pool::instance().render(skin_name,template_name,out,content);
				return;
			}
		}
		// reload all if needed
		booster::unique_lock<booster::recursive_shared_mutex> lock(d->lock);
		for(size_t i=0;i<d->skins.size();i++) {
			impl::skin &current = d->skins[i];
			time_t mtime = impl::get_mtime(current.file_name);
			if(mtime == current.mtime) {
				continue;
			}
			BOOSTER_DEBUG("cppcms") << "Reloading shared object/dll " << current.file_name;
			current.so.reset();
			current.mtime = mtime;
			current.so.reset(new impl::shared_object(current.file_name,true));
		}
		pool::instance().render(skin_name,template_name,out,content);
		return;
	}
	pool::instance().render(skin_name,template_name,out,content);
}


} // views
} // cppcms
