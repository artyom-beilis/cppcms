#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <algorithm>
#include "cppcms_error.h"
#include "aes_encryptor.h"

using namespace std;

namespace cppcms {

namespace aes {

namespace {
class load {
	public:
	load() {
		gcry_check_version(NULL);
	}
};

} // anon namespace

cipher::cipher(string k) :
	encryptor(k)
{
	if(gcry_cipher_open(&hd,GCRY_CIPHER_AES,GCRY_CIPHER_MODE_CBC,0)<0){
		throw cppcms_error("Create failed");
	}

	if( gcry_cipher_setkey(hd,&key.front(),16) < 0 ) {
		gcry_cipher_close(hd);
		throw cppcms_error("Set Key failed");
	}
}

cipher::~cipher() 
{
	gcry_cipher_close(hd);
}

string cipher::encrypt(string const &plain,time_t timeout)
{
	size_t block_size=(plain.size() + 15) / 16 * 16;

	vector<unsigned char> data(16+sizeof(info)+block_size,0);
	info &header=*(info *)(&data.front()+16);
	header.timeout=timeout;
	header.size=plain.size();
	salt(header.salt);
	
	gcry_md_hash_buffer(GCRY_MD_MD5,&data.front()+16,&data.front(),block_size+sizeof(info));
	gcry_cipher_encrypt(hd,&data.front(),data.size(),NULL,0);
	gcry_cipher_reset(hd);
	
	return base64_enc(data);
}

bool cipher::decrypt(string const &cipher,string &plain,time_t *timeout)
{
	if(cipher.size() % 16!=0) return false;
	if(cipher.size()<16+sizeof(info)) return false;

	vector<char> data(cipher.begin(),cipher.end());
	
	gcry_cipher_decrypt(hd,&data.front(),data.size(),NULL,0);
	gcry_cipher_reset(hd);
	vector<char> md5(16,0);
	gcry_md_hash_buffer(GCRY_MD_MD5,&md5.front(),&data.front()+16,data.size()-16);
	if(!std::equal(md5.begin(),md5.end(),data.begin())) {
		return false;
	}
	info &header=*(info *)(&data.front()+16);
	time_t now;
	time(&now);
	if(now>header.timeout)
		return false;
	if(timeout) *timeout=header.timeout;
	plain.assign(data.begin()+16+sizeof(info),data.end());
	return true;
}


} // namespace aes

} // namespace cppcms


