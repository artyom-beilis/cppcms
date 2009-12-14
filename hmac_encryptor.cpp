#define CPPCMS_SOURCE
#include "hmac_encryptor.h"
#include "md5.h"
#include <time.h>

using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {

hmac_cipher::hmac_cipher(string key) :
	base_encryptor(key)
{
}

void hmac_cipher::hash(unsigned char const *data,size_t size,unsigned char md5[16])
{
	vector<unsigned char> ipad(16,0),opad(32,0);
	for(unsigned i=0;i<16;i++) {
		ipad[i]=0x36 ^ key[i];
		opad[i]=0x5c ^ key[i];
	}
	using namespace cppcms::impl;
	md5_state_t state;
	md5_init(&state);
	md5_append(&state,&ipad.front(),16);
	md5_append(&state,data,size);
	md5_finish(&state,&opad.front()+16);
	md5_init(&state);
	md5_append(&state,&opad.front(),32);
	md5_finish(&state,md5);
}

string hmac_cipher::encrypt(string const &plain,time_t timeout)
{
	vector<unsigned char> data(16+sizeof(info)+plain.size(),0);
	info &header=*(info *)(&data.front()+16);
	header.timeout=timeout;
	header.size=plain.size();
	salt(header.salt);
	copy(plain.begin(),plain.end(),data.begin()+16+sizeof(info));
	hash(&data.front()+16,data.size()-16,&data.front());
	return base64_enc(data);
}

bool hmac_cipher::decrypt(string const &cipher,string &plain,time_t *timeout)
{
	vector<unsigned char> data;
	base64_dec(cipher,data);
	const unsigned offset=16+sizeof(info);
	if(data.size()<offset)
		return false;
	info &header=*(info *)(&data.front()+16);
	if(header.size!=data.size()-offset)
		return false;
	unsigned char md5[16];
	hash(&data.front()+16,data.size()-16,md5);
	if(!equal(data.begin(),data.begin()+16,md5))
		return false;
	time_t now;
	time(&now);
	if(now>header.timeout)
		return false;
	if(timeout)
		*timeout=header.timeout;
	plain.assign(data.begin()+offset,data.end());
	return true;
}


} // impl
} // sessions
} // cppcms
