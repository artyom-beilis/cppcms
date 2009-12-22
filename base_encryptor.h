#ifndef CPPCMS_ENCRYPTOR_H
#define CPPCMS_ENCRYPTOR_H

#include <vector>
#include <string>

#include "session_cookies.h"
#include "urandom.h"

namespace cppcms {
namespace sessions {
namespace impl {

class base_encryptor : public cppcms::sessions::encryptor {
public:
	virtual std::string encrypt(std::string const &plain,time_t timeout) = 0;
	virtual bool decrypt(std::string const &cipher,std::string &plain,time_t *timeout=NULL) = 0;
	base_encryptor(std::string key);
	virtual ~base_encryptor();
protected:
	unsigned rand(unsigned );
	std::vector<unsigned char> key;
	std::string base64_enc(std::vector<unsigned char> const &data);
	void base64_dec(std::string const &,std::vector<unsigned char> &data);
	struct info {
		int64_t timeout;
		uint16_t size;
		char salt[6];
	};
	void salt(char *s);
private:
	#ifdef CPPCMS_WIN_NATIVE
	urandom_device rnd;
	#endif
	unsigned seed;
};

} // impl
} // sessions
} // cppcms

#endif
