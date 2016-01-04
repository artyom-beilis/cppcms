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

namespace cppcms {
namespace plugin {

class CPPCMS_API signature_error : public booster::bad_cast {
public:
	signature_error(std::string const &msg);
	~signature_error() throw();
	virtual char const *what() const throw();
private:
	std::string msg_;
};

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
	///   booster::callback<cppcms::application *(cppcms::service &)> cb = cppcms::plugin::manager::entry<cppcms::application *(cppcms::service &)>("foo","application");
	///   cppcms::application *app =cb(service());
	///   attach(app,"/plugins/foo(/.*)",1); // attach new application
	/// \endcode
	/// 
	/// Or
	///
	/// \code
	///   cppcms::application *app = cppcms::plugin::manager::entry<cppcms::application *(cppcms::service &)>("myapi","app::generator")(service());
	///   attach(app,"/plugins/foo(/.*)",1);
	/// \endcode
	///
	/// \ver{v1_2}
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
#define CPPCMS_NAMED_PLUGIN_ENTRY(plugin_name,call_name,call,type,signature)		\
namespace {										\
	struct CPPCMS_PLUGIN_CONCAT2(stpg_ , __LINE__) {				\
		static booster::intrusive_ptr<booster::refcounted> entry()		\
		{									\
			typedef booster::callback<type> ct;				\
			ct cb = &call;							\
			booster::refcounted *tmp = cb.get_pointer().get();		\
			booster::intrusive_ptr<booster::refcounted> ptr(tmp);		\
			return ptr;							\
		}									\
		CPPCMS_PLUGIN_CONCAT2(stpg_,__LINE__) () {				\
			cppcms::plugin::manager::instance().add_entry(			\
				plugin_name,call_name,&entry,signature			\
			);								\
											\
		}									\
		~CPPCMS_PLUGIN_CONCAT2(stpg_,__LINE__)() {				\
			cppcms::plugin::manager::instance().remove_entry(&entry);	\
		}									\
	} CPPCMS_PLUGIN_CONCAT2(instance_of_stpg_,__LINE__);				\
} 

#define CPPCMS_PLUGIN_ENTRY(name,call,type) CPPCMS_NAMED_PLUGIN_ENTRY(#name,#call,name :: call,type,#type)


} // plugin
} // cppcms

#endif
