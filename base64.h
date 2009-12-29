#ifndef CPPCMS_BASE64_H
#define CPPCMS_BASE64_H

#include "defs.h"
#include <string>


namespace cppcms {
	/// 
	/// \brief this namespace provides functions useful for modified Base64 encoding for URL.
	/// This encoding does not insert newline characters, do not pad the text with = character and
	/// use "_" and "-" instead of "+" and "/" characters reserved by URL format for special purposes.
	///
namespace b64url {
	
	///
	/// Calculate required buffer size of base64-url compatible encoding for source of size \a s
	///
	int CPPCMS_API encoded_size(size_t s);
	///
	/// Calculate required buffer size of base64-url compatible decoding for source of size \a s
	///
	/// Note, if original size is invalid, negative value is returned
	///
	int CPPCMS_API decoded_size(size_t s);

	///
	/// Perform base64 URL encoding of the binary data in range [\a begin,\a end), and store it to output buffer
	/// \a target. The size of target storage should have a capacity calculated with encoded_size(end-begin).
	///
	/// Pointer to the first character directly after text string ends is returned.
	///
	unsigned char CPPCMS_API *encode(unsigned char const *begin,unsigned char const *end,unsigned char *target);

	///
	/// Perform base64 URL decoding of the binary data in range [\a begin,\a end), and store it to output buffer
	/// \a target. The size of target storage should have a capacity calculated with encoded_size(end-begin).
	///
	/// Pointer to the first character directly after text string ends is returned. Invalid codes are substituted
	/// by 0 values.
	///
	///
	unsigned char CPPCMS_API *decode(unsigned char const *begin,unsigned char const *end,unsigned char *target);
}
}


#endif
