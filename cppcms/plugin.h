///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2016  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_PLUGIN_H
#define CPPCMS_PLUGIN_H
#include <cppcms/defs.h>
#include <cppcms/cppcms_error.h>
#include <booster/callback.h>
#include <booster/hold_ptr.h>
#include <string>
#include <set>

namespace booster { class shared_object; }

namespace cppcms {
namespace json { class value; }
///
/// \brief Plugin related API
///
/// \ver{v1_2}
namespace plugin {

///
/// An exception that is thrown in case of actual function signature is not matching the requested one
///
/// \ver{v1_2}
class CPPCMS_API signature_error : public booster::bad_cast {
public:
	signature_error(std::string const &msg);
	~signature_error() throw();
	virtual char const *what() const throw();
private:
	std::string msg_;
};


///
/// Class that esures that plugin is loaded and unloads it in destructor if needed
///
/// Note: it tracks the loaded plugins by its name globally such that if another scope had loaded the plugin 
/// it wouldn't be loaded again.
///
/// It is useable when plugin should be used outside of life scope of cppcms::service
///
/// CppCMS configuration:
///
/// - The search paths defined as array of strings in `plugin.paths` (optional)
/// - List of modules defined as array of strings in `plugin.modules` (optional, if you want to call load later)
/// - Shared object pattern defined as string in `plugin.shared_object_pattern` (optional)
///
/// \ver{v1_2}
class CPPCMS_API scope {
	scope(scope const &);
	void operator=(scope const &);
public:
	///
	/// Create an empty scope
	///
	scope();
	///
	/// Unloads all loaded plugins
	///
	~scope();

	///
	/// Loads the plugins provided in main cppcms configuration file - argc,argv are same parameters as for cppcms::service constructor
	///
	scope(int argc,char **argv);
	///
	/// Loads the plugins provided in main cppcms configuration json file - same parameters as for cppcms::service constructor
	///
	scope(json::value const &value);

	///
	/// Set search path for plugins if undefined search according to the OS rules, if one of the paths in the vector is empty the search is performed by 
	//// OS search rules
	///
	void paths(std::vector<std::string> const &paths);
	///
	/// Specify shared object/DLL naming convension. For example `lib{1}.dll` or `lib{1}.so` for converting the module name to shared object/dll name.
	///
	/// Thus in the shared object \a pattern is `lib{1}.dll` that when module "foo" is loaded it tries to load `libfoo.dll` If not speficied default
	/// nameing is used, see booster::shared_object::name
	///
	void shared_object_pattern(std::string const &pattern);

	///
	/// Load specific module according to the paths and shared_object_pattern provided. Also note paths and pattern can be defined in cppcms configuration
	/// in the constructor
	///
	/// \note  module name isn't nessary same as plugin name. Module refers to name of shared object or dll while plugin is application defined. Same dll/so can 
	/// contain multiple plugins or none.
	///
	void load(std::string const &module);

	///
	/// Check if the module was loaded withing any of the scopes - note it is static member function
	///
	static bool is_loaded(std::string const &module);

	///
	/// Get shared object loading withing \a this scope. If it wasn't loaded withing this scope throws cppcms_error
	///
	booster::shared_object const &get(std::string const &module) const;

	///
	/// Check if module is loaded withing this scope, unlike is_loaded that checks for the module globally, it refers to this scope only
	///
	bool is_loaded_by_this_scope(std::string const &module) const;
private:
	void init(json::value const &config);
	struct _class_data;
	static _class_data &class_data();

	struct _data;
	booster::hold_ptr<_data> d;
};


///
/// Central class that manages registration of plugins. 
///
/// It is used as singleton and accessed via manager::instance().
///
/// Each plugin registers itself in the constructor and destructor implemented in shared library.
///
///
/// \ver{v1_2}
///
class CPPCMS_API manager {
public:
	///
	/// Get the instance of the manager
	///
	static manager &instance();

	typedef booster::intrusive_ptr<booster::refcounted> refcounted_ptr;
	///
	/// Functions registered as plugin entry points
	///
	typedef refcounted_ptr (*entry_point_type)();

	///
	/// Get plugin entry by \a plugin_name, \a entry_name and \a Signature
	///
	/// If entry is not found or no entry point is created throws cppcms_error,if  Signature mismatches the callback type
	/// throws booster::bad_cast
	///
	/// For example
	/// \code
	///   booster::callback<cppcms::application *(cppcms::service &)> cb = :manager::instance().entry<cppcms::application *(cppcms::service &)>("foo","application");
	///   cppcms::application *app =cb(service());
	///   attach(app,"/plugins/foo(/.*)",1); // attach new application
	/// \endcode
	/// 
	/// Or
	///
	/// \code
	///   cppcms::application *app = manager::instance().entry<cppcms::application *(cppcms::service &)>("myapi","app::generator")(service());
	///   attach(app,"/plugins/foo(/.*)",1);
	/// \endcode
	///
	template<typename Signature>
	booster::callback<Signature>
	entry(std::string const &plugin_name,std::string const &entry_name)
	{
		typedef booster::callback<Signature> callback_type;
		typedef typename callback_type::callable_type callable_type;
		typedef typename callback_type::pointer_type pointer_type;

		entry_point_type plugin_call = instance().get_entry(plugin_name,entry_name);
		if(!plugin_call)
			throw cppcms_error("Could not find entry `" + entry_name + "' in plugin `" + plugin_name + "'");
		refcounted_ptr call = plugin_call();
		if(!call)
			throw cppcms_error("Failed to create callback from plugin `"+plugin_name+"':entry `" + entry_name + "'");

		callable_type *real_call = dynamic_cast<callable_type *>(call.get());
		if(!real_call) {
			throw signature_error("Invalid signature request in plugin `"+ plugin_name +"':entry `"+entry_name+"', expected following signaure `" + instance().signature(plugin_name,entry_name) + "'");
		}
		pointer_type ptr(real_call);
		callback_type result(ptr);
		return result;
	}

	///
	/// Check if plugin entry of type Signature exists in a plugin \a plugin named \a name
	///
	template<typename Signature>
	bool has_entry(std::string const &plugin,std::string const &name)
	{
		typedef booster::callback<Signature> callback_type;

		entry_point_type plugin_call = get_entry(plugin,name);
		if(!plugin_call)
			return false;
		return dynamic_cast<callback_type *>(plugin_call().get())!=0;
	}

	///
	/// Get entry point that creates a base of booster::callback::callable_type
	///
	entry_point_type get_entry(std::string const &plugin,std::string const &name);
	///
	/// Get textual representation of entry point signature - for logging purposes, if not found returns empty string
	///
	std::string signature(std::string const &plugin,std::string const &name);

	///
	///  Addes entry to the plugin manager - thread safe function
	///
	void add_entry(char const *plugin_name,char const *entry_name,entry_point_type entry,char const *signature);
	///
	/// Removes entry from the plugin manager - thread safe function 
	///
	void remove_entry(entry_point_type entry);

	///
	/// Get list of all plugin names
	///
	std::set<std::string> plugins();
	///
	/// Get list of all entry names for \a plugin
	///
	std::set<std::string> entries(std::string const &plugin);

	///
	/// Returns true if plugin \a name is loaded
	///
	bool has_plugin(std::string const &name);

private:
	manager();
	~manager();
	manager(manager const &);
	void operator=(manager const &);

	struct _data;
	struct entry_type;
	booster::hold_ptr<_data> d;
};

#define CPPCMS_PLUGIN_CONCAT(x,y) x ## y
#define CPPCMS_PLUGIN_CONCAT2(x,y) CPPCMS_PLUGIN_CONCAT(x,y)

///
/// Install generic plugin entry in plugin named \a plugin_name - string, the entry name \a entry_name - string
/// and such that following expression is valid callback iniitalization:
/// `booster::callback<type> cb = call;`
///
/// For example
/// \code
/// class my_class : public plugin_api {
/// public:
///    statuc my_class *create(std::string const &parameter) { return new my_class(parameter); }
///    ...
/// };
/// CPPCMS_PLUGIN_ENTRY("myplugin","api",&my_class::create,plugin_api *(std::string const &))
/// }
/// \endcode
/// 
/// it is accessed as `manager::instance().entry<plugin_api *(std::string const &)>("myplugin","my_class::create")`
///
/// \relates cppcms::plugin::manager
///
#define CPPCMS_FULL_PLUGIN_ENTRY(plugin_name,entry_name,call,type)			\
namespace {										\
	struct CPPCMS_PLUGIN_CONCAT2(stpg_ , __LINE__) {				\
		static booster::intrusive_ptr<booster::refcounted> entry()		\
		{									\
			typedef booster::callback<type> ct;				\
			ct cb = call;							\
			booster::refcounted *tmp = cb.get_pointer().get();		\
			booster::intrusive_ptr<booster::refcounted> ptr(tmp);		\
			return ptr;							\
		}									\
		CPPCMS_PLUGIN_CONCAT2(stpg_,__LINE__) () {				\
			cppcms::plugin::manager::instance().add_entry(			\
				plugin_name,entry_name,&entry,#type			\
			);								\
											\
		}									\
		~CPPCMS_PLUGIN_CONCAT2(stpg_,__LINE__)() {				\
			cppcms::plugin::manager::instance().remove_entry(&entry);	\
		}									\
	} CPPCMS_PLUGIN_CONCAT2(instance_of_stpg_,__LINE__);				\
} 


///
/// Install common function entry such that \a name is plugin name, \a call is entry name and `&name::call` is valid assignment
/// for booster::callback<type>
///
/// Usually name should be namespace or class name, call is function or static member functions
///
/// For example
/// \code
/// namespace myplugin {
///	class my_class : public plugin_api {
///     public:
///         statuc my_class *create(std::string const &parameter) { return new my_class(parameter); }
///         ...
///     };
///     CPPCMS_PLUGIN_ENTRY(myplugin,my_class::create,plugin_api *(std::string const &))
/// }
/// \endcode
/// 
/// it is accessed as `manager::instance().entry<plugin_api *(std::string const &)>("myplugin","my_class::create")`
///
/// \relates cppcms::plugin::manager
///
#define CPPCMS_PLUGIN_ENTRY(name,call,type) CPPCMS_FULL_PLUGIN_ENTRY(#name,#call,& name :: call,type)

///
/// Install common function entry such that \a name is plugin name, \a entry is entry name and `&name::call` is valid assignment
/// for booster::callback<type>
///
/// Usually name should be namespace or class name, call is function or static member functions
///
/// For example
/// \code
/// namespace myplugin {
///	class my_class : public plugin_api {
///     public:
///         statuc my_class *create(std::string const &parameter) { return new my_class(parameter); }
///         ...
///     };
///     CPPCMS_NAMED_PLUGIN_ENTRY(myplugin,api,my_class::create,plugin_api *(std::string const &))
/// }
/// \endcode
/// 
/// it is accessed as `manager::instance().entry<plugin_api *(std::string const &)>("myplugin","api")`
///
/// \relates cppcms::plugin::manager
///
#define CPPCMS_NAMED_PLUGIN_ENTRY(name,entry,call,type) CPPCMS_FULL_PLUGIN_ENTRY(#name,#entry,& name :: call,type)


} // plugin
} // cppcms

#endif
