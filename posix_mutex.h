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
#ifndef CPPCMS_POSIX_MUTEX_H
#define CPPCMS_POSIX_MUTEX_H

#include <pthread.h>

namespace cppcms {

class mutex_lock {
	pthread_mutex_t &m;
public:
	mutex_lock(pthread_mutex_t &p): m(p) { pthread_mutex_lock(&m); };
	~mutex_lock() { pthread_mutex_unlock(&m); };
};

class rwlock_rdlock {
	pthread_rwlock_t &m;
public:
	rwlock_rdlock(pthread_rwlock_t &p): m(p) { pthread_rwlock_rdlock(&m); };
	~rwlock_rdlock() { pthread_rwlock_unlock(&m); };
};

class rwlock_wrlock {
	pthread_rwlock_t &m;
public:
	rwlock_wrlock(pthread_rwlock_t &p): m(p) { pthread_rwlock_wrlock(&m); };
	~rwlock_wrlock() { pthread_rwlock_unlock(&m); };
};

}

#endif
