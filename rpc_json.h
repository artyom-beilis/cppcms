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
#ifndef CPPCMS_RPC_JSON_OBJECT_H
#define CPPCMS_RPC_JSON_OBJECT_H

#include "application.h"

namespace cppcms {
namespace rpc {

	class CPPCMS_API json_call : public refcounted {
	public:

		json_call(intrusive_ptr<http::context> context);
		json_call(http::context &context);
		~json_call();

		std::string method();
		json::array const &params();
		bool is_notification();

		json::value &result();
		json::value &error();

		void on_drop(util::callback0 const &handler)

		void submit();

	private:
		intrusive_ptr<http::context> context_;

		struct data;
		util::hold_ptr<data> d;
	};

	#define CPPCMS_JSON_RPC_BINDER(N)						\
	private:									\
	template<typename Class,typename Result CPPCMS_JSON_TEMPLATE_PARAMS>		\
	struct binder##N : public call {						\
		binder##N(Class *o=0,Result (Class::*m=0)(CPPCMS_JSON_TYPE_PARAMS)):	\
			object(o),							\
			member(m)							\
		{									\
		}									\
		virtual call *clone() const { return new binder##N(*this); }		\
		virtual json::value operator()(json::array const &a) const		\
		{									\
			if(a.size()!=N) throw call_error("Invalid parameres number");	\
			return (object->*member)(CPPCMS_JSON_CALL_PARAMS);		\
		}									\
		Class *object;								\
		Result (Class::*member)(CPPCMS_JSON_TYPE_PARAMS);			\
	};										\
	public:

		

	class json_method {
	public:
		json::value operator()(json::array const &a) const
		{
		}
	private:
		
		struct call {
			virtual call *clone() const = 0;
			virtual json::value operator()(json::array const &) const = 0;
			virtual ~call(){}
		};
		#define CPPCMS_JSON_TEMPLATE_PARAMS 
		#define CPPCMS_JSON_TYPE_PARAMS
		#define CPPCMS_JSON_CALL_PARAMS
		CPPCMS_JSON_RPC_BINDER(0)
		#undef CPPCMS_JSON_TEMPLATE_PARAMS
		#undef CPPCMS_JSON_TYPE_PARAMS
		#undef CPPCMS_JSON_CALL_PARAMS

		#define CPPCMS_JSON_TEMPLATE_PARAMS ,typename P1
		#define CPPCMS_JSON_TYPE_PARAMS  P1
		#define CPPCMS_JSON_CALL_PARAMS  a[0]
		CPPCMS_JSON_RPC_BINDER(1)
		#undef CPPCMS_JSON_TEMPLATE_PARAMS
		#undef CPPCMS_JSON_TYPE_PARAMS
		#undef CPPCMS_JSON_CALL_PARAMS

		#define CPPCMS_JSON_TEMPLATE_PARAMS ,typename P1,typename P2
		#define CPPCMS_JSON_TYPE_PARAMS  P1,P2
		#define CPPCMS_JSON_CALL_PARAMS  a[0],a[1]
		CPPCMS_JSON_RPC_BINDER(2)
		#undef CPPCMS_JSON_TEMPLATE_PARAMS
		#undef CPPCMS_JSON_TYPE_PARAMS
		#undef CPPCMS_JSON_CALL_PARAMS

		#define CPPCMS_JSON_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3
		#define CPPCMS_JSON_TYPE_PARAMS  P1,P2,P3
		#define CPPCMS_JSON_CALL_PARAMS  a[0],a[1],a[2]
		CPPCMS_JSON_RPC_BINDER(3)
		#undef CPPCMS_JSON_TEMPLATE_PARAMS
		#undef CPPCMS_JSON_TYPE_PARAMS
		#undef CPPCMS_JSON_CALL_PARAMS



	
		util::clone_ptr<call> callback_;
	};

	class json_object : public application {
	public:
		typedef intrusive_ptr<json::call> json_ptr;
		
		void add_method(std::string name,util::callback1<json_ptr> handler);
		void add_method_description(std::string name,json::value const &schema);

		template<typename Class,typename Pointer>
		void add_method(std::string name,void (Class::*member)(json_ptr),Pointer object)
		{
			add_method(name,json_details::binder0<Class,Pointer>(member,object));

			json::value description;
			description.set("type","method");
			description.set("params",json::array());
			add_method_description(name,description);
		}
		template<typename Class,typename Pointer,typename P1>
		void add_method(std::string name,void (Class::*member)(json_ptr,P1 const &),Pointer object)
		{
			add_method(name,json_details::binder1<Class,Pointer,P1>(member,object));
			description.set("type","method");
			description.set("params",json::array());
			add_method_description(name,description);
		}

	};



} // rpc
} // cppcms



#endif
