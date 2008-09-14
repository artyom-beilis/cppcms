#ifndef TCP_CACHE_PROTO_H
#define TCP_CACHE_PROTO_H

#include <stdint.h>

#ifdef __cplusplus
namespace cppcms {

namespace opcodes {
#endif
	enum {	fetch_page, fetch, rise, clear, store ,stats,
		error, done, page_data, data, no_data, out_stats };
#ifdef __cplusplus
}
#endif

struct tcp_operation_header {
	uint32_t opcode;
	uint32_t size;
	uint32_t filler[2];
	union {
		struct {
			uint32_t gzip;
			uint32_t strlen;
		} fetch_page;
		struct {
			uint32_t key_len;
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
			uint32_t strlen;
		} page_data;
		struct {
			uint32_t data_len;
			uint32_t triggers_len;
		} data;
		struct {
			uint32_t keys;
			uint32_t triggers;
		} out_stats;
	} operations;
};

#ifdef __cplusplus
} // Namespace cppcms
#endif


#endif
