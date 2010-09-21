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
#include <sstream>
#include <cppcms/cppcms_error.h>
#include "aes_encryptor.h"

#include <cppcms/base64.h>
#include <cppcms/crypto.h>
#include <string.h>
#include <time.h>


using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {

aes_cipher::aes_cipher(std::string k,std::string name) 
{
	using cppcms::impl::aes_api;
	if(name == "aes" || name == "aes-128" || name == "aes128") {
		type_ = aes_api::aes128;
	}
	else if(name == "aes-192" || name == "aes192")  {
		type_ = aes_api::aes192;
	}
	else if(name == "aes-256" || name == "aes256") {
		type_ = aes_api::aes256;
	}
	else {
		throw cppcms_error("Unsupported AES algorithm `" + name + "', supported are aes,aes-128, aes-192, aes-256");
	}

	if(k.size() != unsigned(type_ / 4) ) {
		std::ostringstream ss;
		ss << "AES: requires key length for algorithm " << name << " is " << type_/8
		   << " bytes (" << type_/4 << "hexadecimal digits)";
		throw cppcms_error(ss.str());
	}

	key_ = to_binary(k);
}

aes_cipher::~aes_cipher()
{
}

void aes_cipher::load()
{
	if(!api_.get())
		api_ = cppcms::impl::aes_api::create(type_,key_);
}

std::string aes_cipher::encrypt(string const &plain,time_t timeout)
{
	load();

	size_t block_size=(plain.size() + 15) / 16 * 16;

	std::vector<unsigned char> data(sizeof(aes_hdr)+sizeof(info)+block_size,0);
	copy(plain.begin(),plain.end(),data.begin() + sizeof(aes_hdr)+sizeof(info));
	aes_hdr &aes_header=*(aes_hdr*)(&data.front());
	info &header=*(info *)(&data.front()+sizeof(aes_hdr));
	header.timeout=timeout;
	header.size=plain.size();
	memset(&aes_header,0,16);

	std::auto_ptr<message_digest> md(message_digest::md5());
	md->append(&header,block_size+sizeof(info));
	md->readout(&aes_header.md5);

	std::vector<unsigned char> odata(data.size(),0);
	api_->encrypt(&data.front(),&odata.front(),data.size());

	return base64_enc(odata);
}

bool aes_cipher::decrypt(string const &cipher,string &plain,time_t *timeout)
{
	load();
	vector<unsigned char> idata;
	base64_dec(cipher,idata);
	size_t norm_size=b64url::decoded_size(cipher.size());
	if(norm_size<sizeof(info)+sizeof(aes_hdr) || norm_size % 16 !=0)
		return false;

	vector<unsigned char> data(idata.size(),0);
	api_->decrypt(&idata.front(),&data.front(),data.size());
	
	char md5[16];
	std::auto_ptr<message_digest> md(message_digest::md5());
	md->append(&data.front()+sizeof(aes_hdr),data.size()-sizeof(aes_hdr));
	md->readout(md5);
	
	aes_hdr &aes_header = *(aes_hdr*)&data.front();
	if(memcmp(md5,aes_header.md5,16)!=0) {
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


