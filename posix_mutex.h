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
