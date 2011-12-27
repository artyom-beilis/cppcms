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
#ifndef TCP_CACHE_PROTO_H
#define TCP_CACHE_PROTO_H

#include <cppcms/cstdint.h>

namespace cppcms {
namespace impl {
	namespace opcodes {
		enum {	fetch, rise, clear, store ,stats,
			error, done, data, no_data, uptodate, out_stats,
			session_save, session_load, session_load_data, session_remove};
		inline char const *to_name(int v)
		{
			char const *names[] = {
				"fetch","rise", "clear", "store" ,"stats",
				"error", "done", "data", "no_data", "uptodate", "out_stats",
				"session_save", "session_load", "session_load_data", "session_remove"
			};
			if(v<0 || v>session_remove) 
				return "unknown";
			return names[v];
		}
	}
	struct tcp_operation_header {
		uint32_t opcode;
		uint32_t size;
		uint32_t filler[2];
		union {
			struct {
				uint64_t current_gen;
				uint32_t key_len;
				uint32_t transfer_triggers : 1;
				uint32_t transfer_if_not_uptodate : 1;
				uint32_t reserver : 30;
			} fetch;
			struct {
				uint32_t trigger_len;
			} rise;
			struct {
				uint32_t key_len;
				uint32_t data_len;
				uint32_t triggers_len;
				uint32_t timeout;
			} store;
			struct {
				uint64_t generation;
				uint32_t data_len;
				uint32_t triggers_len;
				uint32_t timeout;
			} data;
			struct {
				uint32_t keys;
				uint32_t triggers;
			} out_stats;
			struct {
				uint32_t timeout;
			} session_save;
			struct {
				uint32_t timeout;
			} session_data;
		} operations;
	};

} // namespace impl
} // Namespace cppcms


#endif
