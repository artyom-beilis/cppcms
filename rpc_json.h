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

	namespace json_details {
		typedef intrusive_ptr<json::call> json_ptr;

		template<typename Class,typename Pointer>
		struct binder0 {
		public:
			typedef Pointer pointer_type;
			typedef (Class::*member_type)(json_ptr);
		
			binder0(member_type m,pointer_type o) : object_(o), member_(m) {}


			void operator()(json_ptr p) const
			{
				if(p->params.size()!=0) {
					throw json::bad_value_cast(); 
				}
				((*object_).*member_)(p);
			}
		private:
			pointer_type object_;
			member_type meber_;
		};
		
		template<typename Class,typename Pointer,typename P1>
		struct binder1 {
		public:
			typedef Pointer pointer_type;
			typedef (Class::*member_type)(json_ptr,P1 const &);
		
			binder1(member_type m,pointer_type o) : object_(o), member_(m) {}


			void operator()(json_ptr p) const
			{
				if(p->params.size()!=1) {
					throw json::bad_value_cast(); 
				}
				P1 p1=p->params[0].get<P1>();
				((*object_).*member_)(p,p1);
			}
		private:
			pointer_type object_;
			member_type meber_;
		};
		
		template<typename Class,typename Pointer,typename P1,typename P2>
		struct binder2 {
		public:
			typedef Pointer pointer_type;
			typedef (Class::*member_type)(json_ptr,P1 const &,P2 const &);
		
			binder2(member_type m,pointer_type o) : object_(o), member_(m) {}


			void operator()(json_ptr p) const
			{
				if(p->params.size()!=2) {
					throw json::bad_value_cast(); 
				}
				P1 p1=p->params[0].get<P1>();
				P2 p2=p->params[1].get<P2>();
				((*object_).*member_)(p,p1,p2);
			}
		private:
			pointer_type object_;
			member_type meber_;
		};

		template<typename Class,typename Pointer,typename P1,typename P2,typename P3>
		struct binder3 {
		public:
			typedef Pointer pointer_type;
			typedef (Class::*member_type)(json_ptr,P1 const &,P2 const &,P3 const &);
		
			binder3(member_type m,pointer_type o) : object_(o), member_(m) {}


			void operator()(json_ptr p) const
			{
				if(p->params.size()!=3) {
					throw json::bad_value_cast(); 
				}
				P1 p1=p->params[0].get<P1>();
				P2 p2=p->params[1].get<P2>();
				P3 p3=p->params[2].get<P3>();
				((*object_).*member_)(p,p1,p2,p3);
			}
		private:
			pointer_type object_;
			member_type meber_;
		};

		template<typename Class,typename Pointer,typename P1,typename P2,typename P3,typename P4>
		struct binder4 {
		public:
			typedef Pointer pointer_type;
			typedef (Class::*member_type)(json_ptr,P1 const &,P2 const &,P3 const &,P4 const &);
		
			binder2(member_type m,pointer_type o) : object_(o), member_(m) {}


			void operator()(json_ptr p) const
			{
				if(p->params.size()!=3) {
					throw json::bad_value_cast(); 
				}
				P1 p1=p->params[0].get<P1>();
				P2 p2=p->params[1].get<P2>();
				P3 p3=p->params[2].get<P3>();
				P4 p4=p->params[3].get<P4>();
				((*object_).*member_)(p,p1,p2,p3,p4);
			}
		private:
			pointer_type object_;
			member_type meber_;
		};
	
	} // json details

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
