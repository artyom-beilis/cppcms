#ifndef CPPCMS_UTIL_REGEX_H
#define CPPCMS_UTIL_REGEX_H

#include "defs.h"
#include "noncopyable.h"
#include "hold_ptr.h"

namespace cppcms { namespace util {

	class regex_result;

	///
	/// This class is actually used as a simple wrapper of \a boost::regex,
	/// \a smatch and \a match_results.
	///
	/// It is created in order to provide stable ABI and not depend on specific
	/// version of Boost. It mostly used as a simple API for regular expressions.
	///
	
	
	class CPPCMS_API regex : public noncopyable {
	public:
		///
		/// Creates new object that is used to match a regular \a expression.
		/// It is actually wrapper of boost::regex.
		///
		
		regex(std::string const &expression);

		~regex();

		///
		/// This function checks if string \a str matches the regular expression.
		/// If so, it stores the result in \a res and returns true, otherwise returs
		/// false. It actually calls \a boost::regex::match_result
		/// 
		bool match(std::string const &str,regex_result &res) const;

	private:
		struct data;
		hold_ptr<data> d;
	};

	///
	/// This class holds result of matching regular expression \a regex. It is
	/// actually wrapper of \a boost::regex::smatch
	///

	class CPPCMS_API regex_result : public noncopyable {
	public:
		regex_result();
		~regex_result();
		///
		/// This operator returns matched expression at position \a n
		///
		std::string operator[](int n);
	private:
		friend class regex;
		struct data;
		hold_ptr<data> d;
	};

}} // cppcms::util


#endif
