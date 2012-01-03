///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
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
