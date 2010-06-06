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
#ifndef CPPCMS_ARCHIVE_TRAITS_H
#define CPPCMS_ARCHIVE_TRAITS_H

#include <cppcms/serialization_classes.h>
#include <cppcms/json.h>

#include <booster/shared_ptr.h>
#include <booster/intrusive_ptr.h>
#include <booster/hold_ptr.h>
#include <booster/clone_ptr.h>
#include <booster/copy_ptr.h>

#include <booster/traits/enable_if.h>
#include <booster/traits/is_base_of.h>

#include <memory>
#include <iterator>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <sstream>

/// \cond internal

namespace cppcms {

		
	template<typename S>
	struct archive_traits<S,typename booster::enable_if<booster::is_base_of<serializable_base,S> >::type > {
		static void save(serializable_base const &d,archive &a)
		{
			d.save(a);
		}
		static void load(serializable_base &d,archive &a)
		{
			d.load(a);
		}
	};

	template<typename T>
	struct archive_traits<T const> {
		static void save(T const &o,archive &a) 
		{
			archive_traits<T>::save(o,a);
		}
		static void load(T &o,archive &a)
		{
			archive_traits<T>::load(o,a);
		}
	};
	
	template<>
	struct archive_traits<std::string> {
		static void save(std::string const &o,archive &a) 
		{
			a.write_chunk(o.c_str(),o.size());
		}
		static void load(std::string &o,archive &a)
		{
			std::string res = a.read_chunk_as_string();
			res.swap(o);
		}
	};

	namespace details {

		struct pod_data {
			void *pointer;
			size_t len;
		};

		struct const_pod_data {
			void const *pointer;
			size_t len;
		};

		inline archive &operator<<(archive &a,const_pod_data const &d)
		{
			a.write_chunk(d.pointer,d.len);
			return a;
		}

		
		inline archive &operator&(archive &a,pod_data const &d)
		{
			if(a.mode()==archive::load_from_archive)
				a.read_chunk(d.pointer,d.len);
			else
				a.write_chunk(d.pointer,d.len);
			return a;
		}
		inline archive &operator<<(archive &a,pod_data const &d)
		{
			a.write_chunk(d.pointer,d.len);
			return a;
		}
		inline archive &operator>>(archive &a,pod_data const &d)
		{
			a.read_chunk(d.pointer,d.len);
			return a;
		}

	}

	template<typename T>
	details::pod_data as_pod(T &v)
	{
		details::pod_data d = { &v , sizeof(v) };
		return d;
	}
	
	template<typename T>
	details::const_pod_data as_pod(T const &v)
	{
		details::const_pod_data d = { &v , sizeof(v) };
		return d;
	}
	
} // cppcms


#define CPPCMS_TRIVIAL_ARCHIVE(Type)				\
namespace cppcms {						\
	template<>						\
	struct archive_traits<std::vector<Type> > {		\
		typedef std::vector<Type> vec;			\
		static void save(vec const &v,archive &a)	\
		{						\
			void const *p=0;			\
			size_t len=				\
				v.size()*sizeof(Type);		\
			if(!v.empty())				\
				p=&v.front();			\
			a.write_chunk(p,len);			\
		}						\
		static void load(vec &v,archive &a)		\
		{						\
			size_t n=a.next_chunk_size() / 		\
				sizeof(Type);			\
			v.clear();				\
			v.resize(n);				\
			void *p=0;				\
			if(!v.empty())				\
				p=&v.front();			\
			a.read_chunk(p,n*sizeof(Type));		\
		}						\
	};							\
								\
	template<int n>						\
	struct archive_traits<Type[n]> {			\
		static void save(Type const d[n],archive &a)	\
		{						\
			a.write_chunk(&d[0],sizeof(Type)*n);	\
		}						\
		static void load(Type d[n],archive &a)		\
		{						\
			a.read_chunk(&d[0],sizeof(Type)*n);	\
		}						\
	};							\
	template<int n>						\
	struct archive_traits<Type const [n]> {			\
		static void save(Type const d[n],archive &a)	\
		{						\
			a.write_chunk(&d[0],sizeof(Type)*n);	\
		}						\
	};							\
								\
	template<>						\
	struct archive_traits<Type> {				\
		static void save(Type const d,archive &a)	\
		{						\
			a.write_chunk(&d,sizeof(d));		\
		}						\
		static void load(Type &d,archive &a)		\
		{						\
			a.read_chunk(&d,sizeof(d));		\
		}						\
	};							\
} /* cppcms */						

CPPCMS_TRIVIAL_ARCHIVE(char)
CPPCMS_TRIVIAL_ARCHIVE(signed char)
CPPCMS_TRIVIAL_ARCHIVE(unsigned char)
CPPCMS_TRIVIAL_ARCHIVE(signed short)
CPPCMS_TRIVIAL_ARCHIVE(unsigned short)
CPPCMS_TRIVIAL_ARCHIVE(signed int)
CPPCMS_TRIVIAL_ARCHIVE(unsigned int)
CPPCMS_TRIVIAL_ARCHIVE(signed long)
CPPCMS_TRIVIAL_ARCHIVE(unsigned long)
CPPCMS_TRIVIAL_ARCHIVE(signed long long)
CPPCMS_TRIVIAL_ARCHIVE(unsigned long long)
CPPCMS_TRIVIAL_ARCHIVE(wchar_t)
CPPCMS_TRIVIAL_ARCHIVE(float)
CPPCMS_TRIVIAL_ARCHIVE(double)
CPPCMS_TRIVIAL_ARCHIVE(long double)


namespace cppcms {

	template<typename T,int size>
	struct archive_traits<T [size]> 
	{
		static void save(T const d[size],archive &a)
		{
			for(int i=0;i<size;i++)
				archive_traits<T>::save(d[i],a);
		}
		static void load(T d[size],archive &a)
		{
			for(int i=0;i<size;i++)
				archive_traits<T>::load(d[i],a);
		}
	};

	template<typename F,typename S>
	struct archive_traits<std::pair<F,S> > 
	{
		static void save(std::pair<F,S> const &d,archive &a)
		{
			archive_traits<F>::save(d.first,a);
			archive_traits<S>::save(d.second,a);
		}
		static void load(std::pair<F,S> &d,archive &a)
		{
			archive_traits<F>::load(d.first,a);
			archive_traits<S>::load(d.second,a);
		}
	};

	template<>
	struct archive_traits<json::value> {
		static void save(json::value const &v,archive &a)
		{
			std::ostringstream ss;
			v.save(ss);
			std::string s=ss.str();
			a.write_chunk(s.c_str(),s.size());
		}
		static void load(json::value &v,archive &a)
		{
			std::istringstream ss;
			ss.str(a.read_chunk_as_string());
			if(!v.load(ss,true)) {
				throw archive_error("Invalid json");
			}
		}
	};

	namespace details {
		
		template<typename T>
		void archive_save_container(T const &v,archive &a)
		{
			typename T::const_iterator it;
			typedef typename T::value_type value_type;
			size_t n=v.size();
			archive_traits<size_t>::save(n,a);
			for(it=v.begin();it!=v.end();++it) {
				archive_traits<value_type>::save(*it,a);
			}
		}

		template<typename T>
		void archive_load_container(T &v,archive &a)
		{
			size_t n;
			archive_traits<size_t>::load(n,a);
			v.clear();
			std::insert_iterator<T> it(v,v.begin());
			typedef typename T::value_type value_type;
			for(size_t i=0;i<n;i++) {
				value_type tmp;
				archive_traits<value_type>::load(tmp,a);
				*it++ = tmp;
			}
		}
	} // details



} /// cppcms

#define CPPCMS_CONTAINER_ARCHIVE2(Type)					\
namespace cppcms {							\
	template<typename V1,typename V2>				\
	struct archive_traits<Type<V1,V2> >				\
	{								\
		typedef Type<V1,V2> cont;				\
		static void save(cont const &d,archive &a)		\
		{							\
			details::archive_save_container<cont>(d,a);	\
		}							\
		static void load(cont &v,archive &a)			\
		{							\
			size_t n;					\
			archive_traits<size_t>::load(n,a);		\
			v.clear();					\
			typedef std::pair<V1,V2> pair_type;		\
			for(size_t i=0;i<n;i++) {			\
				pair_type tmp;				\
				archive_traits<pair_type>::load(tmp,a);	\
				v.insert(tmp);				\
			}						\
		}							\
	};								\
} /* cppcms */
	
#define CPPCMS_CONTAINER_ARCHIVE(Type) 					\
namespace cppcms {							\
	template<typename V>						\
	struct archive_traits<Type<V> >					\
	{								\
		static void save(Type<V> const &d,archive &a)		\
		{							\
			details::archive_save_container<Type<V> >(d,a);	\
		}							\
		static void load(Type<V> &d,archive &a)			\
		{							\
			details::archive_load_container<Type<V> >(d,a);	\
		}							\
	};								\
} /* cppcms */

CPPCMS_CONTAINER_ARCHIVE(std::vector)
CPPCMS_CONTAINER_ARCHIVE(std::list)
CPPCMS_CONTAINER_ARCHIVE2(std::map)
CPPCMS_CONTAINER_ARCHIVE2(std::multimap)
CPPCMS_CONTAINER_ARCHIVE(std::set)
CPPCMS_CONTAINER_ARCHIVE(std::multiset)

#define CPPCMS_ARCHIVE_SMART_POINTER(SmartPtr)			\
namespace cppcms {						\
	template<typename V>					\
	struct archive_traits<SmartPtr<V> > {			\
		typedef SmartPtr<V> pointer;			\
		static void save(pointer const &d,archive &a)	\
		{						\
			char empty = d.get() == 0;		\
			a.write_chunk(&empty,1);		\
			if(!empty) {				\
				archive_traits<V>::save(*d,a);	\
			}					\
		}						\
		static void load(pointer &d,archive &a)		\
		{						\
			char empty;				\
			a.read_chunk(&empty,1);			\
			if(empty) {				\
				d.reset();			\
			}					\
			else {					\
				d.reset(new V());		\
				archive_traits<V>::load(*d,a);	\
			}					\
		}						\
	};							\
} /* cppcms */


#define CPPCMS_ARCHIVE_INTRUSIVE_POINTER(SmartPtr)		\
namespace cppcms {						\
	template<typename V>					\
	struct archive_traits<SmartPtr<V> > {			\
		typedef SmartPtr<V> pointer;			\
		static void save(pointer const &d,archive &a)	\
		{						\
			char empty = d.get() == 0;		\
			a.write_chunk(&empty,1);		\
			if(!empty) {				\
				archive_traits<V>::save(*d,a);	\
			}					\
		}						\
		static void load(pointer &d,archive &a)		\
		{						\
			char empty;				\
			a.read_chunk(&empty,1);			\
			if(empty) {				\
				d = 0;				\
			}					\
			else {					\
				d = new V();			\
				archive_traits<V>::load(*d,a);	\
			}					\
		}						\
	};							\
} /* cppcms */




CPPCMS_ARCHIVE_SMART_POINTER(booster::shared_ptr)
CPPCMS_ARCHIVE_SMART_POINTER(booster::hold_ptr)
CPPCMS_ARCHIVE_SMART_POINTER(booster::copy_ptr)
CPPCMS_ARCHIVE_SMART_POINTER(booster::clone_ptr)
CPPCMS_ARCHIVE_SMART_POINTER(std::auto_ptr)
CPPCMS_ARCHIVE_INTRUSIVE_POINTER(booster::intrusive_ptr)



/// \endcond


#endif
