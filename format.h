#ifndef CPPCMS_UTIL_FORMAT_H
#define CPPCMS_UTIL_FORMAT_H

#include "defs.h"
#include "noncopyable.h"

#include <iostream>
#include <string>
#include <iterator>

namespace cppcms { namespace util {

	class CPPCMS_API format_iterator : public util::noncopyable {
		std::ostream &out_;
		char const *current_,*end_;
	public:
		format_iterator(std::ostream &out,std::string const &format);
		~format_iterator();
		int write();
		static const int end=-1;
	};

#ifdef CPPCMS_HAVE_VARIADIC_TEMPLATES
	namespace details {
		inline void format_nth(std::ostream &out,int n)
		{
		}
		template<typename A,typename... Args>
		inline void format_nth(std::ostream &out,int n,A v,Args... args)
		{
			if(n==1) out<<v;
			else format_nth(out,n-1,args...);
		}
	} // details
	

	template<typename... Args>
	inline std::ostream &format(std::ostream &out,std::string const &s,Args... args)
	{
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			details::format_nth(out,n,args...);
		}
		return out;
	}

#else
	inline std::ostream &format(std::ostream &out,std::string const &form)
	{		
		format_iterator f(out,form);
		int n;
		while((n=f.write())!=format_iterator::end) {
			// Nothing
		}
		return out;
	}

	template<typename T1>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			}
		}
		return out;
	}

	template<typename T1,typename T2>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break;
			case 2: out<<p2; break;
			}
		}
		return out;
	}

	template<typename T1,typename T2,typename T3>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2,T3 p3)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			case 2: out<<p2; break; 
			case 3: out<<p3; break; 
			}
		}
		return out;
	}

	template<typename T1,typename T2,typename T3,typename T4>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2,T3 p3,T4 p4)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			case 2: out<<p2; break; 
			case 3: out<<p3; break; 
			case 4: out<<p4; break; 
			}
		}
		return out;
	}

	template<typename T1,typename T2,typename T3,typename T4,typename T5>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2,T3 p3,T4 p4,T5 p5)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			case 2: out<<p2; break; 
			case 3: out<<p3; break; 
			case 4: out<<p4; break; 
			case 5: out<<p5; break; 
			}
		}
		return out;
	}

	template<typename T1,typename T2,typename T3,typename T4,typename T5,typename T6>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2,T3 p3,T4 p4,T5 p5,T6 p6)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			case 2: out<<p2; break; 
			case 3: out<<p3; break; 
			case 4: out<<p4; break; 
			case 5: out<<p5; break; 
			case 6: out<<p6; break; 
			}
		}
		return out;
	}

	template<typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2,T3 p3,T4 p4,T5 p5,T6 p6,T7 p7)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			case 2: out<<p2; break; 
			case 3: out<<p3; break; 
			case 4: out<<p4; break; 
			case 5: out<<p5; break; 
			case 6: out<<p6; break; 
			case 7: out<<p7; break; 
			}
		}
		return out;
	}

	template<typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2,T3 p3,T4 p4,T5 p5,T6 p6,T7 p7,T8 p8)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			case 2: out<<p2; break; 
			case 3: out<<p3; break; 
			case 4: out<<p4; break; 
			case 5: out<<p5; break; 
			case 6: out<<p6; break; 
			case 7: out<<p7; break; 
			case 8: out<<p8; break; 
			}
		}
		return out;
	}

	template<typename T1,typename T2,typename T3,typename T4,typename T5,typename T6,typename T7,typename T8,typename T9>
	inline std::ostream &format(std::ostream &out,std::string const &s,T1 p1,T2 p2,T3 p3,T4 p4,T5 p5,T6 p6,T7 p7,T8 p8,T9 p9)
	{		
		format_iterator f(out,s);
		int n;
		while((n=f.write())!=format_iterator::end) {
			switch(n) {
			case 1: out<<p1; break; 
			case 2: out<<p2; break; 
			case 3: out<<p3; break; 
			case 4: out<<p4; break; 
			case 5: out<<p5; break; 
			case 6: out<<p6; break; 
			case 7: out<<p7; break; 
			case 8: out<<p8; break; 
			case 9: out<<p9; break; 
			}
		}
		return out;
	}
#endif


} } // cppcms::util

#endif
