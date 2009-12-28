#define CPPCMS_SOURCE
#include "base_encryptor.h"
#include "base64.h"
#include "cppcms_error.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {

base_encryptor::~base_encryptor()
{
}

base_encryptor::base_encryptor(string key_):
	key(16,0)

{
	if(key_.size()!=32) {
		throw cppcms_error("Incorrect key length (32 expected)\n");
	}
	for(unsigned i=0;i<32;i+=2) {
		char buf[3];
		if(!isxdigit(key_[i]) || !isxdigit(key_[i+1])) {
			throw cppcms_error("Cipher should be encoded as hexadecimal 32 digits number");
		}
		buf[0]=key_[i];
		buf[1]=key_[i+1];
		buf[2]=0;
		unsigned v;
		sscanf(buf,"%x",&v);
		key[i/2]=v;
	}
	urandom_device dev;
	dev.generate(&seed,sizeof(seed));
}

unsigned base_encryptor::rand(unsigned max)
{
	#ifdef CPPCMS_WIN_NATIVE
	unsigned res;
	rnd.generate(&res,sizeof(res));
	return (unsigned)(res/(RAND_MAX+1.0)*max);
	#else
	return (unsigned)(rand_r(&seed)/(RAND_MAX+1.0)*max);
	#endif
}

string base_encryptor::base64_enc(vector<unsigned char> const &data)
{
	size_t size=b64url::encoded_size(data.size());
	vector<unsigned char> result(size,0);
	b64url::encode(&data.front(),&data.front()+data.size(),&result.front());
	return string(result.begin(),result.end());
}

void base_encryptor::base64_dec(std::string const &in,std::vector<unsigned char> &data)
{
	int size=b64url::decoded_size(in.size());
	if(size<0) return;
	data.resize(size);
	unsigned char const *ptr=(unsigned char const *)in.data();
	b64url::decode((unsigned char const *)ptr,ptr+in.size(),&data.front());
}

void base_encryptor::salt(char *salt)
{
	info dummy;
	#ifdef CPPCMS_WIN_NATIVE
	unsigned res;
	rnd.generate(salt,sizeof(dummy.salt));
	#else
	for(unsigned i=0;i<sizeof(dummy.salt);i++)
		salt[i]=rand(256);
	#endif
}

} // impl
} // sessions
} // cppcms
