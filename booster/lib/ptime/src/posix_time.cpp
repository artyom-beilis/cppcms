//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOSTER_SOURCE
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <booster/posix_time.h>
#include <booster/ctime.h>

#ifdef BOOSTER_WIN_NATIVE
#include <windows.h>
#else
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <iostream>

namespace booster {

const ptime ptime::zero = ptime(0);

ptime ptime::now()
{
	#ifndef BOOSTER_WIN_NATIVE
	struct timeval tv;
	gettimeofday(&tv,0);
	return ptime(tv.tv_sec,tv.tv_usec * 1000);
	#else
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	unsigned long long tt = ft.dwHighDateTime;
	tt <<=32;
	tt |= ft.dwLowDateTime;
	tt /=10;
	tt -= 11644473600000000ULL;
	return ptime(ptime::microseconds(tt));
	#endif
}

void ptime::sleep(ptime const &p)
{
	#ifndef BOOSTER_WIN_NATIVE
	struct timespec ts,tsr;
	ts.tv_sec = p.sec;
	ts.tv_nsec = p.nsec;
	while(::nanosleep(&ts,&tsr) < 0 && errno==EINTR)
		ts=tsr;
	#else
	ptime n = now();
	ptime end = n + p;
	while(end > n) {
		::Sleep(DWORD(milliseconds(end - n)));
		n = now();
	}
	#endif
}

std::tm ptime::universal_time(ptime const &p)
{
	return booster::universal_time(p.sec);
}

std::tm ptime::local_time(ptime const &p)
{
	return booster::local_time(p.sec);
}

ptime ptime::universal_time(std::tm const &t)
{
	return ptime(booster::make_universal_time(t));
}

ptime ptime::local_time(std::tm const &t)
{
	return ptime(booster::make_local_time(t));
}


std::ostream &operator<<(std::ostream &out,ptime const &p)
{
	out << ptime::to_number(p);
	return out;
}
std::istream &operator>>(std::istream &in,ptime &p)
{
	double v;
	in >> v;
	if(!in.fail())
		p=ptime::from_number(v);
	return in;
}

}
