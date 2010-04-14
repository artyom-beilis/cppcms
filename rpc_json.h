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
#include "refcounted.h"
#include "intrusive_ptr.h"
#include "function.h"
#include "json.h"

namespace cppcms {
namespace rpc {

	class CPPCMS_API call_error : public cppcms_error {
	public:	
		call_error(std::string const &message);
	};

	class json_rpc_server;

	class CPPCMS_API json_call : public refcounted {
	public:

		json_call(http::context &context);
		~json_call();

		std::string method();
		bool notification();

		json::array const &params();

		http::context &context();

		void return_result(json::value const &);
		void return_error(json::value const &);
	
	private:
		friend class json_rpc_server;
		void return_result(http::context &,json::value const &);
		void return_error(http::context &,json::value const &);
		void attach_context(intrusive_ptr<http::context> context);

		void check_not_notification();
		intrusive_ptr<http::context> context_;
		json::value  id_;
		json::array params_;
		std::string method_;
		bool notification_;

		struct data;
		util::hold_ptr<data> d;
	};

	class CPPCMS_API json_rpc_server : public application {
	public:
		typedef enum {
			any_role,
			method_role,
			notification_role
		} role_type;
		typedef function<void(json::array const &)> method_type;
		void bind(std::string const &name,method_type const &,role_type type = any_role);
		virtual void main(std::string);

		intrusive_ptr<json_call> release_call();
		
		json_rpc_server(cppcms::service &srv);
		~json_rpc_server();

		std::string method();
		bool notification();
		json::array const &params();
		void return_result(json::value const &);
		void return_error(json::value const &);
	private:
		void check_call();
		struct data;
		struct method_data {
			method_type method;
			role_type role;
		};
		typedef std::map<std::string,method_data> methods_map_type;
		methods_map_type methods_;
		intrusive_ptr<json_call> current_call_;
		util::hold_ptr<data> d;
	};


	#define CPPCMS_JSON_RPC_BINDER(N)						\
	namespace details {								\
		template<typename Class,typename Ptr CPPCMS_TEMPLATE_PARAMS>		\
		struct binder##N {							\
			Ptr object;							\
			void (Class::*member)(CPPCMS_FUNC_PARAMS);			\
			void operator()(json::array const &a) const			\
			{								\
				if(a.size()!=N) 					\
					throw call_error("Invalid parametres number");	\
				((*object).*member)(CPPCMS_CALL_PARAMS);		\
			}								\
		};									\
	}										\
	template<typename Class,typename Ptr CPPCMS_TEMPLATE_PARAMS>			\
	details::binder##N<Class,Ptr CPPCMS_BINDER_PARAMS> 				\
	json_method(void (Class::*m)(CPPCMS_FUNC_PARAMS),Ptr p)				\
	{ details::binder##N<Class,Ptr CPPCMS_BINDER_PARAMS> tmp={p,m}; return tmp; }	\

	#define CPPCMS_TEMPLATE_PARAMS
	#define CPPCMS_FUNC_PARAMS
	#define CPPCMS_CALL_PARAMS
	#define CPPCMS_BINDER_PARAMS
	CPPCMS_JSON_RPC_BINDER(0)
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_FUNC_PARAMS
	#undef CPPCMS_CALL_PARAMS
	#undef CPPCMS_BINDER_PARAMS

	#define CPPCMS_TEMPLATE_PARAMS ,typename P1
	#define CPPCMS_FUNC_PARAMS P1
	#define CPPCMS_CALL_PARAMS  a[0].get_value<P1>()
	#define CPPCMS_BINDER_PARAMS ,P1
	CPPCMS_JSON_RPC_BINDER(1)
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_FUNC_PARAMS
	#undef CPPCMS_CALL_PARAMS
	#undef CPPCMS_BINDER_PARAMS
	
	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2
	#define CPPCMS_FUNC_PARAMS P1,P2
	#define CPPCMS_CALL_PARAMS  a[0].get_value<P1>(),a[1].get_value<P2>()
	#define CPPCMS_BINDER_PARAMS ,P1,P2
	CPPCMS_JSON_RPC_BINDER(2)
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_FUNC_PARAMS
	#undef CPPCMS_CALL_PARAMS
	#undef CPPCMS_BINDER_PARAMS

	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3
	#define CPPCMS_FUNC_PARAMS P1,P2,P3
	#define CPPCMS_CALL_PARAMS  a[0].get_value<P1>(),a[1].get_value<P2>(),a[2].get_value<P3>()
	#define CPPCMS_BINDER_PARAMS ,P1,P2,P3
	CPPCMS_JSON_RPC_BINDER(3)
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_FUNC_PARAMS
	#undef CPPCMS_CALL_PARAMS
	#undef CPPCMS_BINDER_PARAMS

	#define CPPCMS_TEMPLATE_PARAMS ,typename P1,typename P2,typename P3,typename P4
	#define CPPCMS_FUNC_PARAMS P1,P2,P3,P4
	#define CPPCMS_CALL_PARAMS  a[0].get_value<P1>(),a[1].get_value<P2>(),a[2].get_value<P3>(), \
				a[3].get_value<P4>()
	#define CPPCMS_BINDER_PARAMS ,P1,P2,P3,P4
	CPPCMS_JSON_RPC_BINDER(4)
	#undef CPPCMS_TEMPLATE_PARAMS
	#undef CPPCMS_FUNC_PARAMS
	#undef CPPCMS_CALL_PARAMS
	#undef CPPCMS_BINDER_PARAMS

	#undef CPPCMS_JSON_RPC_BINDER

} // rpc
} // cppcms



#endif
