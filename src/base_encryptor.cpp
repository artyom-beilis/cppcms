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
#include "base_encryptor.h"
#include <cppcms/base64.h>
#include <cppcms/cppcms_error.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

namespace cppcms {
namespace sessions {
namespace impl {

base_encryptor::~base_encryptor()
{
}

string base_encryptor::base64_enc(vector<unsigned char> const &data)
{
	size_t size=b64url::encoded_size(data.size());
	vector<unsigned char> result(size,0);
	if(data.size() > 0) {
		b64url::encode(&data.front(),&data.front()+data.size(),&result.front());
	}
	return string(result.begin(),result.end());
}

void base_encryptor::base64_dec(std::string const &in,std::vector<unsigned char> &data)
{
	int size=b64url::decoded_size(in.size());
	if(size<=0) return;
	data.resize(size);
	unsigned char const *ptr=(unsigned char const *)in.data();
	b64url::decode((unsigned char const *)ptr,ptr+in.size(),&data.front());
}



} // impl
} // sessions
} // cppcms
