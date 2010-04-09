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
#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <algorithm>
#include "cppcms_error.h"
#include "aes_encryptor.h"

#include "base64.h"

#ifdef CPPCMS_WIN_NATIVE
#define CPPCMS_GCRYPT_USE_BOOST_THREADS
#endif

#ifdef CPPCMS_GCRYPT_USE_BOOST_THREADS
#ifdef CPPCMS_USE_EXTERNAL_BOOST
#   include <boost/thread.hpp>
#else // Internal Boost
#   include <cppcms_boost/thread.hpp>
    namespace boost = cppcms_boost;
#endif
static int nt_mutex_init(void **p)
{
	try {
		*p=new boost::mutex();
		return 0;
	}
	catch(...)
	{
		return 1;
	}
}
static int nt_mutex_destroy(void **p)
{
	boost::mutex **m=reinterpret_cast<boost::mutex **>(p);
	delete *m;
	*m=0;
	return 0;
}

static int nt_mutex_lock(void **p)
{
	boost::mutex *m=reinterpret_cast<boost::mutex *>(*p);
	try {
		m->lock();
		return 0;
	}
	catch(...)
	{
		return 1;
	}
}

static int nt_mutex_unlock(void **p)
{
	boost::mutex *m=reinterpret_cast<boost::mutex *>(*p);
	try {
		m->unlock();
		return 0;
	}
	catch(...)
	{
		return 1;
	}
}

static struct gcry_thread_cbs threads_nt = { 
	GCRY_THREAD_OPTION_USER,
	0,
	nt_mutex_init,
	nt_mutex_destroy,
	nt_mutex_lock,
	nt_mutex_unlock,
	0,0,0,0,
	0,0,0,0 
};

static void set_gcrypt_cbs()
{
	gcry_control (GCRYCTL_SET_THREAD_CBS, &threads_nt);
}
						
#else

#include <pthread.h>
#include <errno.h>

GCRY_THREAD_OPTION_PTHREAD_IMPL;

static void set_gcrypt_cbs()
{
	gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
}

#endif


using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {

namespace {
class load {
	public:
	load() {
		set_gcrypt_cbs();
		gcry_check_version(NULL);
	}
} loader;

} // anon namespace

aes_cipher::aes_cipher(string k) :
	base_encryptor(k)
{
	bool in=false,out=false;
	in=gcry_cipher_open(&hd_in,GCRY_CIPHER_AES,GCRY_CIPHER_MODE_CBC,0) == 0;
	out=gcry_cipher_open(&hd_out,GCRY_CIPHER_AES,GCRY_CIPHER_MODE_CBC,0) == 0;
	if(!in || !out){
		goto error_exit;
	}

	if( gcry_cipher_setkey(hd_in,&key.front(),16) != 0) {
		goto error_exit;
	}
	if( gcry_cipher_setkey(hd_out,&key.front(),16) != 0)
		goto error_exit;
	char iv[16];
	gcry_create_nonce(iv,sizeof(iv));
	gcry_cipher_setiv(hd_out,iv,sizeof(iv));
	return;
error_exit:
	if(in) gcry_cipher_close(hd_in);
	if(out) gcry_cipher_close(hd_out);
	throw cppcms_error("AES cipher initialization failed");
}

aes_cipher::~aes_cipher()
{
	gcry_cipher_close(hd_in);
	gcry_cipher_close(hd_out);
}

string aes_cipher::encrypt(string const &plain,time_t timeout)
{
	size_t block_size=(plain.size() + 15) / 16 * 16;

	vector<unsigned char> data(sizeof(aes_hdr)+sizeof(info)+block_size,0);
	copy(plain.begin(),plain.end(),data.begin() + sizeof(aes_hdr)+sizeof(info));
	aes_hdr &aes_header=*(aes_hdr*)(&data.front());
	info &header=*(info *)(&data.front()+sizeof(aes_hdr));
	header.timeout=timeout;
	header.size=plain.size();
	memset(&aes_header,0,16);

	gcry_md_hash_buffer(GCRY_MD_MD5,&aes_header.md5,&header,block_size+sizeof(info));
	gcry_cipher_encrypt(hd_out,&data.front(),data.size(),NULL,0);

	return base64_enc(data);
}

bool aes_cipher::decrypt(string const &cipher,string &plain,time_t *timeout)
{
	vector<unsigned char> data;
	base64_dec(cipher,data);
	size_t norm_size=b64url::decoded_size(cipher.size());
	if(norm_size<sizeof(info)+sizeof(aes_hdr) || norm_size % 16 !=0)
		return false;

	gcry_cipher_decrypt(hd_in,&data.front(),data.size(),NULL,0);
	gcry_cipher_reset(hd_in);
	vector<char> md5(16,0);
	gcry_md_hash_buffer(GCRY_MD_MD5,&md5.front(),&data.front()+sizeof(aes_hdr),data.size()-sizeof(aes_hdr));
	aes_hdr &aes_header = *(aes_hdr*)&data.front();
	if(!std::equal(md5.begin(),md5.end(),aes_header.md5)) {
		return false;
	}
	info &header=*(info *)(&data.front()+sizeof(aes_hdr));
	if(time(NULL)>header.timeout)
		return false;
	if(timeout) *timeout=header.timeout;

	vector<unsigned char>::iterator data_start=data.begin()+sizeof(aes_hdr)+sizeof(info),
			data_end=data_start+header.size;

	plain.assign(data_start,data_end);
	return true;
}


} // impl
} // sessions

} // cppcms


