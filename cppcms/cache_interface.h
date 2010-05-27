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
#ifndef CPPCMS_CACHE_INTERFACE_H
#define CPPCMS_CACHE_INTERFACE_H

#include <string>
#include <set>

#include <cppcms/defs.h>
#include <booster/noncopyable.h>
#include <booster/intrusive_ptr.h>
#include <booster/hold_ptr.h>
#include <cppcms/cstdint.h>

namespace cppcms {

	namespace impl {
		class base_cache;
	}
	namespace http {
		class context;
	};

	template<typename Object>
	struct serialization_traits;

	///
	/// \brief This class is the major gateway of the application to CppCMS caching abilities. Any access too cache
	/// would be done via this class.
	///
	/// CppCMS cache model supports following concepts:
	///
	/// - \a key the unique identification of the object in cache
	/// - \a timeout -- the maximal time the cached object remains valid
	/// - \a trigger -- special key that allows fast invalidation of multiple cache objects.
	///
	/// The first two concepts are quite popular and available in most of Web framework, but the last one is very
	/// unique to CppCMS that gives fine grained cache invalidation tools.
	///
	/// Each time the page is created it automatically receives some triggers during the process of creation. 
	/// When some object is fetched from the cache or stored into it, it adds triggers to the major page. This provides
	/// semi-automatic triggers management.
	/// 
	/// For example:
	/// 
	/// \code
	///   if(cache().fetch_page("main_page"))
	///     return;
	///  if(!cache().fetch_frame("article_"+id,article)) {
	///     article=generate_article_from_data_base(id);
	///	cache.store_frame("article_"+id,article);
	///   }
	///   // Generate some HTML here using article 
	///   cache.store_page("main");
	/// \endcode
	///
	/// Let's assume that "main_page" wasn't found in cache, then we try to fetch a frame that holds only a single
	/// article "article_123", if it is fetched, the result is stored in a string article and the trigger "article_123"
	/// is automatically added to set of triggers that "main_page" depends on them.
	/// 
	/// When the article updated, and "article_123" key is risen, it would automatically invalidate "main_page" as well.
	///
	/// CppCMS cache_interface allows storing arbitrary object in cache, For this purpose they should be "serializable".
	/// This can be done by specializing a class cppcms::setialization_traits
	///
       

	
	class CPPCMS_API cache_interface : public booster::noncopyable {
	public:

		///
		/// Internal API, don't use it
		///
		cache_interface(http::context &context);
		~cache_interface();

		
		///
		/// Rise a trigger \a trigger. All cached objects that depend on this trigger would be invalidated
		///
		void rise(std::string const &trigger);

		///
		/// Add a trigger \a trigger to the list of dependencies of current page.
		///

		void add_trigger(std::string const &trigger);

		///
		/// Clear all CppCMS cache - use carefully
		///
		void clear();

		///
		/// Remove all triggers added to current page so far
		///
		void reset();

		///
		/// Get statistics about items stored in cache. May require O(n) complexity, use with care.
		///
		/// \param keys -- the number of items stored in cache
		/// \param triggers -- the number of various triggers existing in the cache.
		///
		/// Returns false if caching system is disabled.
		///
		bool stats(unsigned &keys,unsigned &triggers);
	
		///
		/// Returns true if caching system is enabled
		///	
		bool has_cache();

		///
		/// Opposite of \a has_cache
		///
		bool nocache();

		///
		/// Fetch a page from the cache with a key \a key. If the page exists, it is written to output
		/// and true is returned. Otherwise false is returned.
		///
		bool fetch_page(std::string const &key);

		///
		/// Store page with key \a akey in cache, with timeout \a timeout.
		///
		/// This function stores a page with dependencies on all triggers that were added so far.
		///
		/// \param key -- the key that defines the cache.
		/// \param timeout -- maximal valid time of the page. \a timeout=-1 means infinite. Use with care.
		///
		/// Note: store_page does not rise the trigger \a key, only replaces the value.
		///

		void store_page(std::string const &key,int timeout=-1);

		///
		/// Fetch a string (usually some HTML part) from the cache.
		///
		/// \param key -- the key that uniquely defines the frame.
		/// \param result -- string to store fetched value
		/// \param notriggers -- if true, no triggers that a frame is dependent on would be added to dependencies of
		///       the current page, otherwise (false, default), the all triggers that page is dependent on, including
		///       the \a key itself would be added as dependent triggers to current rendered page.
		/// \return returns true if the entry was found.
		///
		bool fetch_frame(std::string const &key,std::string &result,bool notriggers=false);
		
		///
		/// Store a string (usually some HTML part) to the cache.
		///
		/// \param key -- the key that uniquely defines the frame.
		/// \param frame -- the actual value
		/// \parma triggers  -- the set of triggers that the key should depend on (\a key is added automatically)
		/// \param timeout -- maximal object lifetime, -1 is infinity
		/// \param notriggers -- if \a notriggers is true no frame dependent triggers would be added to the current
		///     page trigger set. Otherwise (default) current page would depend on the \a key and \a triggers as its
		///     dependent triggers.
		///
		void store_frame(std::string const &key,
				 std::string const &frame,
				 std::set<std::string> const &triggers=std::set<std::string>(),
				 int timeout=-1,
				 bool notriggers=false);

		///
		/// Store a string (usually some HTML part) to the cache.
		///
		/// \param key -- the key that uniquely defines the frame.
		/// \param frame -- the actual value
		/// \param timeout -- maximal object lifetime, -1 is infinity
		/// \param notriggers -- if \a notriggers is true \a key added to the current
		///     page trigger set. Otherwise (default) current page would depend on the \a key
		/// 
		void store_frame(std::string const &key,
				 std::string const &frame,
				 int timeout,
				 bool notriggers=false);

		///
		/// Fetch a serializeable object from the cache.
		///
		/// \param key -- the key that uniquely defines the frame.
		/// \param data -- an object store fetched data
		/// \param notriggers -- if true, no triggers that an object is dependent on would be added to dependencies of
		///       the current page, otherwise (false, default), the all triggers that the object is dependent on, including
		///       the \a key itself would be added as dependent triggers to current rendered page.
		/// \return returns true if the entry was found.
		///
		template<typename Serializable>
		bool fetch_data(std::string const &key,Serializable &data,bool notriggers=false)
		{
			std::string buffer;
			if(!fetch(key,buffer,notriggers))
				return false;
			serialization_traits<Serializable>::load(buffer,data);
			return true;
		}
		///
		/// Store a serializeable object to the cache.
		///
		/// \param key -- the key that uniquely defines the object.
		/// \param date -- the actual object
		/// \parma triggers  -- the set of triggers that the key should depend on (\a key is added automatically)
		/// \param timeout -- maximal object lifetime, -1 is infinity
		/// \param notriggers -- if \a notriggers is true no frame dependent triggers would be added to the current
		///     page trigger set. Otherwise (default) current page would depend on the \a key and \a triggers as its
		///     dependent triggers.
		///

		template<typename Serializable>
		void store_data(std::string const &key,Serializable const &data,
				std::set<std::string> const &triggers=std::set<std::string>(),
				int timeout=-1,bool notriggers=false)
		{
			std::string buffer;
			serialization_traits<Serializable>::save(data,buffer);
			store(key,data,triggers,timeout,notriggers);
		}

		///
		/// Store a serializeable object to the cache.
		///
		/// \param key -- the key that uniquely defines the object.
		/// \param data -- the actual object
		/// \param timeout -- maximal object lifetime, -1 is infinity
		/// \param notriggers -- if \a notriggers is true \a key added to the current
		///     page trigger set. Otherwise (default) current page would depend on the \a key
		/// 

		template<typename Serializable>
		void store_data(std::string const &key,Serializable const &data,int timeout,bool notriggers=false)
		{
			store_data<Serializable>(key,data,std::set<std::string>(),timeout,notriggers);
		}

	private:


		void store(	std::string const &key,
				std::string const &data,
				std::set<std::string> const &triggers,
				int timeout,
				bool notriggers);

		bool fetch(	std::string const &key,
				std::string &buffer,
				bool notriggers);

		struct _data;
		booster::hold_ptr<_data> d;
		http::context *context_;
		std::set<std::string> triggers_;
		booster::intrusive_ptr<impl::base_cache> cache_module_;

		uint32_t page_compression_used_ : 1;
		uint32_t reserved : 31;
	};


}

#endif
