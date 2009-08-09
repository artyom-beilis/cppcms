#ifndef CPPCMS_BASE64_H
#define CPPCMS_BASE64_H

#include "defs.h"
#include <string>


namespace cppcms {
namespace b64url {
	ssize_t CPPCMS_API encoded_size(size_t s);
	ssize_t CPPCMS_API decoded_size(size_t s);
	unsigned char CPPCMS_API *encode(unsigned char const *begin,unsigned char const *end,unsigned char *target);
	unsigned char CPPCMS_API *decode(unsigned char const *begin,unsigned char const *end,unsigned char *target);
}
}


#endif
