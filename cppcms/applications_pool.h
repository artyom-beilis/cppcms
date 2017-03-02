///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_APPLICATIONS_POOL_H
#define CPPCMS_APPLICATIONS_POOL_H

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/hold_ptr.h>
#include <booster/intrusive_ptr.h>
#include <booster/shared_ptr.h>
#include <booster/weak_ptr.h>
#include <booster/enable_shared_from_this.h>

#include <booster/auto_ptr_inc.h>
#include <string>

namespace cppcms {
	class application;
}

namespace booster {
	void CPPCMS_API intrusive_ptr_add_ref(cppcms::application *p);
	void CPPCMS_API intrusive_ptr_release(cppcms::application *p);
	namespace aio {
		class io_service;
	}
}

namespace cppcms {

	class service;
	class mount_point;
	class application_specific_pool;
	class applications_pool;
	namespace http {
		class context;
	}

	///
	/// \brief Flags for application pool management
	///	
	/// \ver{v1_2}
	namespace app {
		static const int synchronous 	= 0x0000; ///< Synchronous application 
		static const int asynchronous	= 0x0001; ///< Asynchronous application that operates in asynchronous mode

		static const int op_mode_mask	= 0x000F; ///< mask to select sync vs async flags

		static const int thread_specific= 0x0010; ///< Make synchronous application thread specific
		static const int prepopulated	= 0x0020; ///< Make sure all applications are created from the beginning (ignored in thread_specific is set)
		static const int content_filter = 0x0040; ///< Make this asynchronous application to handle content
		/// \cond INTERNAL
		static const int legacy		= 0x8000; ///< Use legacy handling of application life time when the application is created in the event loop and than dispatched as a job to a thread pool
		/// \endcond
	}

	///
	/// \brief an interface for creating user applications
	///
	/// \ver{v1_2}
	class CPPCMS_API application_specific_pool : 
		public booster::noncopyable,
		public booster::enable_shared_from_this<application_specific_pool>
	{
	public:
		application_specific_pool();
		virtual ~application_specific_pool();

		///
		/// Returns asynchronous application that runs at given booster::aio::io_service constext, it the application
		/// does not exist yet, it is created
		///
		/// Notes:
		///
		/// - if the application is created upon function call it would be created in the calling thread regardless if it is event loop thread or not
		/// - If the pool isn't mounted as asynchronous pool then cppcms_error is thrown
		/// - if the io_srv isn't main cppcms io_service cppcms_error is thrown
		///
		booster::intrusive_ptr<application> asynchronous_application_by_io_service(booster::aio::io_service &io_srv,cppcms::service &srv);
		///
		/// Returns asynchronous application that runs at given booster::aio::io_service constext, it the application
		/// does not exist yet NULL pointer is returned
		///	
		/// Notes:
		///
		/// - If the pool isn't mounted as asynchronous pool then cppcms_error is thrown
		/// - if the io_srv isn't main cppcms io_service cppcms_error is thrown
		///
		booster::intrusive_ptr<application> asynchronous_application_by_io_service(booster::aio::io_service &io_srv);

	protected:
		///
		/// Returns newly created instance of an application, its ownership
		/// is transferred
		///
		virtual application *new_application(service &srv) = 0;
	private:
		int flags();
		void flags(int f);

		void prepopulate(cppcms::service &srv);
		void application_requested(cppcms::service &srv);
		friend class applications_pool;
		friend class application;
		friend class http::context;
		friend void booster::intrusive_ptr_release(cppcms::application *app);

		application *get_new(service &srv);

		void size(size_t n);
		booster::intrusive_ptr<application> get(service &);
		void put(application *app);

		struct _data;
		class _policy;
		class _tls_policy;
		class _pool_policy;
		class _async_policy;
		class _async_legacy_policy;
		class _legacy_pool_policy;
		booster::hold_ptr<_data> d;
	};

	///
	/// \brief Application pool is the central class that holds user created applications
	///
	/// Form the user perspective this class provides an API for mounting user application to the CppCMS service.
	///
	/// There are two kind of \a mount member functions, that allow:
	/// 
	/// - Mounting a \a factory of user applications -- for execution of synchronous requests by multiple
	///   instances of application.
	/// - Mounting single application -- for processing asynchronous requests by single instance of an application
	///
	/// The life cycle of synchronous application is defined by application pool itself, and the life cycle
	/// of asynchronous depends on its own reference count.
	///
	/// This class is thread safe and can be accessed from multiple threads simultaneously.
	///
	class CPPCMS_API applications_pool {
	public:

		///
		/// \brief a base class for user application factories - to be deprecated, use
		/// application_specific_pool instead
		/// 
		/// \deprecated Use application_specific_pool
		///
		struct factory : public booster::noncopyable {
			///
			/// Returns newly created instance of an application.
			///
			virtual std::auto_ptr<application> operator()(service &) const = 0;
			virtual ~factory(){}
		};

		///
		/// Mount an application factory \a aps for processing of any incoming requests. Application
		/// would receive PATH_INFO CGI variable for URL matching.
		///
		/// This member function is thread safe.
		///
		/// \deprecated Use mount(booster::shared_ptr<application_specific_pool> gen,int application_options) instead
		///
		void mount(std::auto_ptr<factory> aps);
		
		///
		/// Mount an application factory \a app  by mount_point \a point application matching and
		/// URL selection rules
		///
		/// This member function is thread safe.
		///
		/// \deprecated Use mount(booster::shared_ptr<application_specific_pool> gen,mount_point const &point,int application_options) instead
		///
		void mount(std::auto_ptr<factory> aps,mount_point const &point);

		///
		/// Mount an asynchronous application \a app for processing of any incoming requests. Application
		/// would receive PATH_INFO CGI variable for URL matching.
		///
		/// This member function is thread safe.
		///
		/// \deprecated Use mount(booster::shared_ptr<application_specific_pool> gen,int application_options) with application_options=app::asynchronous instead
		///
		void mount(booster::intrusive_ptr<application> app);
		///
		/// Mount an asynchronous application \a app  by mount_point \a point application matching and
		/// URL selection rules
		///
		/// This member function is thread safe.
		///
		/// \deprecated Use mount(booster::shared_ptr<application_specific_pool> gen,mount_point const &point,int application_options) with application_options=app::asynchronous instead
		///
		void mount(booster::intrusive_ptr<application> app,mount_point const &point);

		///
		/// Mount a application_specific_pool for an application that processes all requests, path provided to application's main is PATH_INFO
		///
		/// \a application_options allow to specify mode of operation - synchronous, asynchronous, see namespace
		/// cppcms::app
		///
		/// Note: applications_pool owns gen now and is responsible for destroying it
		///
		/// This member function is thread safe.
		///
		/// \ver{v1_2}
		void mount(booster::shared_ptr<application_specific_pool> gen,int application_options = 0);

		///
		/// Mount a application_specific_pool to a specific mount point
		///
		/// \a application_options allow to specify mode of operation - synchronous, asynchronous, see namespace
		/// cppcms::app
		///
		/// Note: applications_pool owns gen now and is responsible for destroying it
		///
		/// This member function is thread safe.
		///
		/// \ver{v1_2}
		void mount(booster::shared_ptr<application_specific_pool> gen,mount_point const &point,int application_options = 0);


		///
		/// Unmount an application_specific_pool from the general pool.
		///
		/// Notes:
		///
		/// - Exiting request would continue to be executed
		/// - There is no guarantee when and in which thread application objects would be destroyed upon use of unmount
		/// - applications in the pool using thread_specific policy would be destroyed only on thread exit (i.e. when threads of thread pool are destroyed)
		///
		/// This member function is thread safe.
		///
		/// \ver{v1_2}
		void unmount(booster::weak_ptr<application_specific_pool> gen);


		/// \cond INTERNAL

		/// get is not in use any more
		booster::intrusive_ptr<application> 
		get(char const *h,char const *s,char const *path_info,std::string &match);

		booster::shared_ptr<application_specific_pool> 
		get_application_specific_pool(char const *h,char const *s,char const *path_info,std::string &match);

		// put is not in use any more
		void put(application *app);
		applications_pool(service &srv,int unused);
		~applications_pool();

		/// \endcond

	private:
		struct _data;
		service *srv_;
		booster::hold_ptr<_data> d;
	};

	/// \cond INTERNAL 
	namespace details {
		template<typename T>
		struct simple_factory0 : public applications_pool::factory
		{
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s));
				return app;
			}
		};
		template<typename T,typename P1>
		struct simple_factory1 : public applications_pool::factory
		{
			simple_factory1(P1 p1) : p1_(p1) {}
			P1 p1_;
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s,p1_));
				return app;
			}
		};
		template<typename T,typename P1,typename P2>
		struct simple_factory2 : public applications_pool::factory 
		{
			simple_factory2(P1 p1,P2 p2) : p1_(p1),p2_(p2) {}
			P1 p1_;
			P2 p2_;
			std::auto_ptr<application> operator()(service &s) const
			{
				std::auto_ptr<application> app(new T(s,p1_,p2_));
				return app;
			}
		};
	} // details

	/// \endcond

	///
	/// Create application factory for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s);
	///
	/// \deprecated Use create_pool
	///
	template<typename T>
	std::auto_ptr<applications_pool::factory> applications_factory()
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory0<T>);
		return f;
	}
	
	///
	/// Create application factory for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s,P1);
	///
	/// \deprecated Use create_pool
	///
	template<typename T,typename P1>
	std::auto_ptr<applications_pool::factory> applications_factory(P1 p1)
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory1<T,P1>(p1));
		return f;
	}
	
	///
	/// Create application factory for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s,P1,P2);
	///
	/// \deprecated Use create_pool
	///
	template<typename T,typename P1,typename P2>
	std::auto_ptr<applications_pool::factory> applications_factory(P1 p1,P2 p2)
	{
		std::auto_ptr<applications_pool::factory> f(new details::simple_factory2<T,P1,P2>(p1,p2));
		return f;
	}

	/// \cond INTERNAL 
	namespace details {
		template<typename T>
		struct simple_application_specific_pool0 : public application_specific_pool
		{
			T *new_application(service &s) 
			{
				return new T(s);
			}
		};
		template<typename T,typename P1>
		struct simple_application_specific_pool1 : public application_specific_pool
		{
			simple_application_specific_pool1(P1 p1) : p1_(p1) {}
			P1 p1_;
			T *new_application(service &s)
			{
				return new T(s,p1_);
			}
		};
		template<typename T,typename P1,typename P2>
		struct simple_application_specific_pool2 : public application_specific_pool 
		{
			simple_application_specific_pool2(P1 p1,P2 p2) : p1_(p1),p2_(p2) {}
			P1 p1_;
			P2 p2_;
			T *new_application(service &s)
			{
				return new T(s,p1_,p2_);
			}
		};
	} // details

	/// \endcond

	///
	/// Create application application_specific_pool for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s);
	///
	template<typename T>
	booster::shared_ptr<application_specific_pool> create_pool()
	{
		booster::shared_ptr<application_specific_pool> f(new details::simple_application_specific_pool0<T>);
		return f;
	}
	
	///
	/// Create application application_specific_pool for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s,P1);
	///
	template<typename T,typename P1>
	booster::shared_ptr<application_specific_pool> create_pool(P1 p1)
	{
		booster::shared_ptr<application_specific_pool> f(new details::simple_application_specific_pool1<T,P1>(p1));
		return f;
	}
	
	///
	/// Create application application_specific_pool for application of type T, such as T has a constructor
	/// T::T(cppcms::service &s,P1,P2);
	///
	template<typename T,typename P1,typename P2>
	booster::shared_ptr<application_specific_pool> create_pool(P1 p1,P2 p2)
	{
		booster::shared_ptr<application_specific_pool> f(new details::simple_application_specific_pool2<T,P1,P2>(p1,p2));
		return f;
	}

} // cppcms



#endif
