///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "hmac_encryptor.h"
#include "md5.h"
#include <time.h>
#include <stdlib.h>

using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {


boost::thread_specific_ptr<hmac_cipher::block> hmac_cipher::seed;

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
	salt(header.salt,sizeof(header.salt));
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


void hmac_cipher::salt(char *salt,int n)
{
	if(!seed.get()) {
		block tmp;
		urandom_device rnd;
		rnd.generate(&tmp,16);
		tmp.left=0;
		seed.reset(new block(tmp));
	}
	block &b=*seed;
	while(n > 0) {
		if(b.left == 0) {
			using namespace cppcms::impl;
			b.seed.counter++;
			md5_state_t md5;
			md5_init(&md5);
			md5_append(&md5,reinterpret_cast<md5_byte_t *>(&b.seed),sizeof(b.seed));
			md5_finish(&md5,reinterpret_cast<md5_byte_t *>(b.buf));
			b.left=sizeof(b.buf);
		}
		*salt++ = b.buf[--b.left];
		n--;
	}
}


} // impl
} // sessions
} // cppcms
