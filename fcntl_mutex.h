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
#ifndef CPPCMS_FCNTL_MUTEX_H
#define CPPCMS_FCNTL_MUTEX_H

#include <fcntl.h>
#include <errno.h>

namespace cppcms {
namespace {
	void do_lock(FILE *f,int how,bool do_throw=false)
	{
		struct flock lock = {0};
		lock.l_type=how;
		lock.l_start=0;
		lock.l_len=0;
		lock.l_whence=SEEK_SET;
		int res;
		while((res=::fcntl(fileno(f),F_SETLKW,&lock))<0 && errno==EINTR)
			;
		if(res!=0 && do_throw)
			throw cppcms_error(errno,"fcntl failed");
	}
}

namespace fcntl {

class mutex_lock {
	FILE *m;
public:
	mutex_lock(FILE *p): m(p) { do_lock(m,F_WRLCK,true); };
	~mutex_lock() {	do_lock(m,F_UNLCK); }
};

class rwlock_rdlock {
	FILE *m;
public:
	rwlock_rdlock(FILE *p): m(p) { do_lock(m,F_RDLCK,true); };
	~rwlock_rdlock() { do_lock(m,F_UNLCK); };
};

class rwlock_wrlock {
	FILE *m;
public:
	rwlock_wrlock(FILE *p): m(p) { do_lock(m,F_WRLCK,true); };
	~rwlock_wrlock() { do_lock(m,F_UNLCK); };
};

}

} // cppcms


#endif
