#ifndef CPPCMS_AES_ENCRYPTOR_H
#define CPPCMS_AES_ENCRYPTOR_H

#include <string>
#include <gcrypt.h>
#include "encryptor.h"

namespace cppcms {

namespace aes {

class cipher : public encryptor {
	gcry_cipher_hd_t hd;
public:
	virtual std::string encrypt(std::string const &plain,time_t timeout) = 0;
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL) = 0;
	cipher(std::string key);
	~cipher();
};

}

}


#endif

