#ifndef CPPCMS_UTIL_NONCOPYABLE_H
#define CPPCMS_UTIL_NONCOPYABLE_H

namespace cppcms { 
namespace util {
	class noncopyable {
	private:
		noncopyable(noncopyable const &);
		noncopyable const &operator=(noncopyable const &);
	protected:
		noncopyable(){}
		~noncopyable(){}
	};
}} // cppcms::util

#endif
