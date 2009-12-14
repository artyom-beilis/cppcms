#ifndef CPPCMS_HMAC_ENCRYPTOR_H
#define CPPCMS_HMAC_ENCRYPTOR_H
#include "base_encryptor.h"

namespace cppcms {
namespace sessions {
namespace impl {

class hmac_cipher : public base_encryptor {
	void hash(unsigned char const *,size_t,unsigned char md5[16]);
public:
	virtual std::string encrypt(std::string const &plain,time_t timeout);
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL);
	hmac_cipher(std::string key);
};

} // impl
} // sessions
} // cppcms


#endif
