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

namespace cppcms {
namespace plugin {


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
	/// Get plugin entry by plugin name, entry point name and type
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
	static
	booster::callback<Signature>
	entry(std::string const &name)
	{
		typedef booster::callback<Signature> callback_type;
		typedef typename callback_type::callable_type callable_type;
		typedef typename callback_type::pointer_type pointer_type;

		entry_point_type plugin_call = instance().get_entry(name);
		if(!plugin_call)
			throw cppcms_error("Could not find plugin `" + name + "'");
		refcounted_ptr call = plugin_call();
		if(!call)
			throw cppcms_error("Failed to create callback from `" + name + "'");

		callable_type *real_call = dynamic_cast<callable_type *>(call.get());
		if(!real_call) {
			throw booster::bad_cast();
		}
		pointer_type ptr(real_call);
		callback_type result(ptr);
		return result;
	}

	///
	/// Get entry point that creates a base of booster::callback::callable_type
	///
	entry_point_type get_entry(std::string const &name);

	///
	///  Addes entry to the plugin manager - thread safe function
	///
	void add_entry(char const *name,entry_point_type entry);
	///
	/// Removes entry from the plugin manager - thread safe function 
	///
	void remove_entry(entry_point_type entry);
private:
	manager();
	~manager();
	manager(manager const &);
	void operator=(manager const &);

	struct _data;
	booster::hold_ptr<_data> d;
};

#define CPPCMS_NAMED_PLUGIN_ENTRY(type,call,name)					\
namespace {										\
	struct stpg_##__LINE__ {							\
		static booster::intrusive_ptr<booster::refcounted> entry()		\
		{									\
			typedef booster::callback<type> ct;				\
			ct cb = &call;							\
			booster::refcounted *tmp = cb.get_pointer().get();		\
			booster::intrusive_ptr<booster::refcounted> ptr(tmp);		\
			return ptr;							\
		}									\
		stpg_##__LINE__() {							\
			cppcms::plugin::manager::instance().add_entry(name,&entry);	\
		}									\
		~stpg_##__LINE__() {							\
			cppcms::plugin::manager::instance().remove_entry(&entry);	\
		}									\
	} instance_of_stpg_##__LINE__;							\
} 

#define CPPCMS_PLUGIN_ENTRY(type,call) CPPCMS_NAMED_PLUGIN_ENTRY(type,call,#call)


} // plugin
} // cppcms

#endif
