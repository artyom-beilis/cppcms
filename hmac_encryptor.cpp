#include "hmac_encryptor.h"
#include "md5.h"
#include <time.h>

using namespace std;

namespace cppcms {
namespace hmac {

cipher::cipher(string key) :
	encryptor(key)
{
}

void cipher::hash(unsigned char const *data,size_t size,unsigned char md5[16])
{
	static unsigned const digest_size = 16;
	static unsigned const block_size = 64;
	vector<unsigned char> ipad(block_size,0),opad(block_size + digest_size,0);
	for(unsigned i=0;i<block_size;i++) {
		ipad[i]=0x36;
		opad[i]=0x5c;
	}
	for(unsigned i=0;i<digest_size;i++) {
		ipad[i] ^= key[i];
		opad[i] ^= key[i];
	}
	md5_state_t state;
	md5_init(&state);
	md5_append(&state,&ipad.front(),block_size);
	md5_append(&state,data,size);
	md5_finish(&state,&opad.front()+block_size);
	md5_init(&state);
	md5_append(&state,&opad.front(),block_size + digest_size);
	md5_finish(&state,md5);
}

string cipher::encrypt(string const &plain,time_t timeout)
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

bool cipher::decrypt(string const &cipher,string &plain,time_t *timeout)
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


} // hmac
} // cppcms
