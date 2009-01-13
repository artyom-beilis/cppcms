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
