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

	///
	/// \brief This class represents POSIX time. 
	///
	/// The time from Jan 1, 1970 in seconds in UTC (without leap seconds) similar to time_t.
	/// 
	/// ptime internally holds 64 bit integer for seconds part and int for nanoseconds
	/// part which gives fine-grained time representation.
	///
	class BOOSTER_API ptime {
	public:
		///
		/// Create the POSIX time from seconds and nanoseconds.
		///
		explicit 
		ptime(long long seconds=0,int nano=0) : 
			sec(seconds),
			nsec(nano) 
		{
			normalize();
		}

		///
		/// Get the seconds part of POSIX time
		///
		long long get_seconds() const
		{
			return sec;
		}
		///
		/// Get the nanoseconds fraction part of POSIX time
		///
		int get_nanoseconds() const
		{
			return nsec;
		}
		///
		/// Get the milliseconds fraction part of POSIX time (which is equal to get_nanoseconds() / 1,000,000)
		///
		int get_milliseconds() const
		{
			return nsec / one_e6;
		}
		///
		/// Get the microseconds fraction part of POSIX time (which is equal to get_nanoseconds() / 1,000)
		///
		int get_microseconds() const
		{
			return nsec / one_e3;
		}

		///
		/// Get amount of full seconds in \a p
		///
		static long long seconds(ptime const &p)
		{
			return p.sec;
		}
		///
		/// Convert a seconds \a v to ptime.
		///
		static ptime seconds(long long v)
		{
			return ptime(v);
		}

		///
		/// Get amount of full milliseconds in \a p.
		/// Not the same as p.get_milliseconds() as takes seconds as well
		///
		static long long milliseconds(ptime const &p) 
		{
			return p.get_seconds() * one_e3 + p.get_milliseconds();
		}
		///
		/// Convert a milliseconds \a v to ptime.
		///
		static ptime milliseconds(long long v)
		{
			return ptime(v/one_e3,v%one_e3 * one_e6);
		}
		///
		/// Get amount of full microseconds in \a p.
		/// Not the same as p.get_microseconds() as takes seconds as well
		///
		static long long microseconds(ptime const &p)
		{
			return p.get_seconds() * one_e6 + p.get_nanoseconds() / one_e3;
		}
		///
		/// Convert a microseconds \a v to ptime.
		///
		static ptime microseconds(long long v)
		{
			return ptime(v/one_e6,v%one_e6 * one_e3);
		}
		///
		/// Get amount of nanoseconds in \a p
		/// Not the same as p.get_nanoseconds() as takes seconds as well
		///
		static long long nanoseconds(ptime const &p)
		{
			return p.get_seconds() * one_e9 + p.get_nanoseconds();
		}
		///
		/// Convert a nanoseconds \a v to ptime.
		///
		static ptime nanoseconds(long long v)
		{
			return ptime(v/one_e9,v%one_e9);
		}
		///
		/// Get amount of full minutes in \a p
		///
		static ptime minutes(long long v)
		{
			return ptime(v*60);
		}
		///
		/// Convert minutes \a v to ptime.
		///
		static long long minutes(ptime const &p)
		{
			return p.get_seconds() / 60;
		}
		///
		/// Get amount of full hours in \a p
		///
		static long long hours(ptime const &p)
		{
			return p.get_seconds() / 3600;
		}
		///
		/// Convert hours \a v to ptime.
		///
		static ptime hours(long long v)
		{
			return ptime(v*3600);
		}
		///
		/// Get amount of full days in \a p
		///
		static long long days(ptime const &p)
		{
			return p.get_seconds() / (3600*24);
		}
		///
		/// Convert days \a v to ptime.
		///
		static ptime days(long long v)
		{
			return ptime(v*(3600*24));
		}
	
		///
		/// Convert \a t to floating point number that represents POSIX time in seconds
		///	
		static double to_number(ptime const &t)
		{
			return double(t.sec) + double(t.nsec) * 1e-9;
		}
		///
		/// Convert floating point number \a d that represents POSIX time in seconds to ptime
		///	
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
		
		///
		/// Add two POSIX time ranges (as numbers)
		///
		ptime operator+(ptime const &other) const
		{
			return ptime(sec+other.sec,nsec+other.nsec);
		}
		///
		/// Subtract one time from other (as number)
		///
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
		
		///
		/// Convert local time to POSIX time similar to mktime
		///
		static ptime local_time(std::tm const &v);
		///
		/// Convert universal time to POSIX time similar to timegm or mktime in GMT timezone
		///
		static ptime universal_time(std::tm const &v);
		///
		/// Convert POSIX time \a v to a local time similar to localtime_r
		///
		static std::tm local_time(ptime const &v);
		///
		/// Convert POSIX time \a v to a GMT time similar to gmtime_r
		///
		static std::tm universal_time(ptime const &v);
		
		///
		/// Get current time
		///
		static ptime now();

		///
		/// Same as ptime() -- 0 in terms of POSIX time
		///
		static ptime const zero;

		///
		/// Sleep at least \a v milliseconds 
		///
		static void millisleep(long long v)
		{
			sleep(milliseconds(v));
		}
		///
		/// Sleep at least \a v nanoseconds 
		///
		static void nanosleep(long long v)
		{
			sleep(nanoseconds(v));
		}
		/// 
		/// Sleep at least \a v amount of time.
		/// 
		static void sleep(ptime const &v );
	
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

	///
	/// Write ptime to stream. It is written as double, so it would give expected result when
	/// working with booster::locale::as::date_time formatter
	///	
	BOOSTER_API std::ostream &operator<<(std::ostream &,ptime const &);
	///
	/// Read ptime from stream. It is read as double, so it would give expected result when
	/// working with booster::locale::as::date_time formatter
	///	
	BOOSTER_API std::istream &operator>>(std::istream &,ptime &);

}

#endif

