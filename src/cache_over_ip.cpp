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
#define CPPCMS_SOURCE
#include "tcp_cache_client.h"
#include <booster/atomic_counter.h>

#include <booster/thread.h>

namespace cppcms {
namespace impl {
	class cache_over_ip : public base_cache {
	public:
		cache_over_ip(std::vector<std::string> ips,std::vector<int> ports,booster::intrusive_ptr<base_cache> l1) :
			ips_(ips),
			ports_(ports),
			l1_(l1),
			refs_(0)
		{
		}
		bool fetch(	std::string const &key,
				std::string *a,std::set<std::string> *tags,
				time_t *timeout_out,
				uint64_t *gen)
		{
			std::string buffer;
			if(!a) a=&buffer;
			time_t tmp_timeout;
			if(!timeout_out) timeout_out=&tmp_timeout;
			uint64_t generation;
			if(!gen) gen=&generation;

			if(!l1_.get()) {
				if(tcp()->fetch(key,*a,tags,*timeout_out,*gen,false)==tcp_cache::found)
					return true;
				return false;
			}

			std::set<std::string> tmp_triggers;

			if(!tags) tags=&tmp_triggers;
			
			if(l1_->fetch(key,a,tags,timeout_out,gen)) {
				int res = tcp()->fetch(key,*a,tags,*timeout_out,*gen,true);
				if(res==tcp_cache::up_to_date)
					return true;
				if(res==tcp_cache::not_found) {
					l1_->remove(key);
					return false;
				}
				l1_->store(key,*a,*tags,*timeout_out,gen);
				return true;
			}
			else {
				if(tcp()->fetch(key,*a,tags,*timeout_out,*gen,false)==tcp_cache::found) {
					l1_->store(key,*a,*tags,*timeout_out,gen);
					return true;
				}
				return false;
			}
		}
		virtual void store(std::string const &key,std::string const &b,std::set<std::string> const &triggers,time_t timeout,uint64_t const * /*gen*/)
		{
			if(l1_.get())
				l1_->remove(key);
			tcp()->store(key,b,triggers,timeout);
		}
		virtual void rise(std::string const &trigger)
		{
			if(l1_.get())
				l1_->rise(trigger);
			tcp()->rise(trigger);
		}
		virtual void remove(std::string const &/*key*/)
		{
			// NA
		}
		virtual void clear()
		{
			if(l1_.get())
				l1_->clear();
			tcp()->clear();
		}
		virtual void stats(unsigned &keys,unsigned &triggers)
		{
			tcp()->stats(keys,triggers);
		}
		virtual void add_ref()
		{
			++refs_;
		}
		virtual bool del_ref()
		{
			if(--refs_ == 0)
				return true;
			return false;
		}
		virtual ~cache_over_ip()
		{
		}

	private:

		tcp_cache *tcp()
		{
			if(!tcp_.get())
				tcp_.reset(new tcp_cache(ips_,ports_));
			return tcp_.get();
		}

		booster::thread_specific_ptr<tcp_cache> tcp_;
		std::vector<std::string> ips_;
		std::vector<int> ports_;
		booster::intrusive_ptr<base_cache> l1_;
		booster::atomic_counter refs_;
	
	};

booster::intrusive_ptr<base_cache> CPPCMS_API 
tcp_cache_factory(	std::vector<std::string> const &ips,
			std::vector<int> const &ports,
			booster::intrusive_ptr<base_cache> l1)
{
	return new cache_over_ip(ips,ports,l1);
}


} // impl
} // cppcms
