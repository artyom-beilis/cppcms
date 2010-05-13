//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_POSIX_TIME_H
#define BOOSTER_POSIX_TIME_H

#include <booster/config.h>
#include <ctime>
#include <iosfwd>
#include <math.h>

namespace booster {
	class BOOSTER_API ptime {
	public:
		explicit 
		ptime(long long seconds=0,int nano=0) : 
			sec(seconds),
			nsec(nano) 
		{
			normalize();
		}

		long long get_seconds() const
		{
			return sec;
		}
		int get_nanoseconds() const
		{
			return nsec;
		}
		int get_milliseconds() const
		{
			return nsec / one_e6;
		}
		int get_microseconds() const
		{
			return nsec / one_e3;
		}

		static long long seconds(ptime const &p)
		{
			return p.sec;
		}
		static ptime seconds(long long v)
		{
			return ptime(v);
		}

		static long long milliseconds(ptime const &p) 
		{
			return p.get_seconds() * one_e3 + p.get_milliseconds();
		}
		static ptime milliseconds(long long v)
		{
			return ptime(v/one_e3,v%one_e3 * one_e6);
		}
		static long long microseconds(ptime const &p)
		{
			return p.get_seconds() * one_e6 + p.get_nanoseconds() / one_e3;
		}
		static ptime microseconds(long long v)
		{
			return ptime(v/one_e6,v%one_e6 * one_e3);
		}
		static long long nanoseconds(ptime const &p)
		{
			return p.get_seconds() * one_e9 + p.get_nanoseconds();
		}
		static ptime nanoseconds(long long v)
		{
			return ptime(v/one_e9,v%one_e9);
		}
		static ptime minutes(long long v)
		{
			return ptime(v*60);
		}
		static long long minutes(ptime const &p)
		{
			return p.get_seconds() / 60;
		}
		static long long hours(ptime const &p)
		{
			return p.get_seconds() / 3600;
		}
		static ptime hours(long long v)
		{
			return ptime(v*3600);
		}
		static long long days(ptime const &p)
		{
			return p.get_seconds() / (3600*24);
		}
		static ptime days(long long v)
		{
			return ptime(v*(3600*24));
		}
		
		static double to_number(ptime const &t)
		{
			return double(t.sec) + double(t.nsec) * 1e-9;
		}
		static ptime from_number(double d)
		{
			double sec = floor(d);
			double subsec = d-sec;
			long long seconds = static_cast<long long>(sec);
			int nano = static_cast<int>(floor(subsec * 1e9));
			if(nano < 0) nano = 0;
			if(nano >= one_e9) nano = one_e9-1;
			return ptime(seconds,nano);
		}
		
		ptime operator+(ptime const &other) const
		{
			return ptime(sec+other.sec,nsec+other.nsec);
		}
		ptime operator-(ptime const &other) const
		{
			return ptime(sec-other.sec,nsec-other.nsec);
		}

		bool operator==(ptime const &other) const
		{
			return sec==other.sec && nsec == other.nsec;
		}
		bool operator!=(ptime const &other) const
		{
			return !((*this)==other);
		}
		bool operator<(ptime const &other) const
		{
			if(sec < other.sec)
				return true;
			if(sec > other.sec)
				return false;
			return nsec < other.nsec;
		}
		bool operator>(ptime const &other) const
		{
			return other < *this;
		}
		bool operator <= (ptime const &other) const
		{
			return !(*this > other);
		}
		bool operator >=(ptime const &other) const
		{
			return !(*this < other);
		}
		
		//
		// static member functions
		// 
		
		static std::tm local_time(ptime const &v);
		static std::tm universal_time(ptime const &v);
		
		static ptime now();

		static ptime const zero;

		static void millisleep(long long v)
		{
			sleep(milliseconds(v));
		}
		static void nanosleep(long long v)
		{
			sleep(nanoseconds(v));
		}
		static void sleep(ptime const &);
	
	private:
		void normalize()
		{
			if(nsec > one_e9) {
				sec += nsec / one_e9;
				nsec = nsec % one_e9;
			}
			else if(nsec < 0) {
				while(nsec < 0) {
					nsec += one_e9;
					sec -= 1;
				}
			}
		}
		static const int one_e3 = 1000;
		static const int one_e6 = 1000000;
		static const int one_e9 = 1000000000;
		long long sec;
		int nsec;
	};
	
	BOOSTER_API std::ostream &operator<<(std::ostream &,ptime const &);
	BOOSTER_API std::istream &operator>>(std::istream &,ptime &);

}

#endif

