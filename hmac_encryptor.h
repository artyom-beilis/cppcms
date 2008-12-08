#ifndef CPPCMS_HMAC_ENCRYPTOR_H
#define CPPCMS_HMAC_ENCRYPTOR_H
#include "encryptor.h"

namespace cppcms {
namespace hmac {

class cipher : public encryptor {
	void hash(unsigned char const *,size_t,unsigned char md5[16]);
public:
	virtual std::string encrypt(std::string const &plain,time_t timeout);
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL);
	cipher(std::string key);
};

} // hmac
} // cppcms


#endif
