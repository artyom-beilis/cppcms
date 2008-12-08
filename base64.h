#ifndef CPPCMS_BASE64_H
#define CPPCMS_BASE64_H

#include <string>

namespace cppcms {
namespace b64url {
	ssize_t encoded_size(size_t s);
	ssize_t decoded_size(size_t s);
	unsigned char *encode(unsigned char const *begin,unsigned char const *end,unsigned char *target);
	unsigned char *decode(unsigned char const *begin,unsigned char const *end,unsigned char *target);
}
}


#endif
