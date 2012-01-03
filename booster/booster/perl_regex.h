//
//  Copyright (C) 2009-2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_PERL_REGEX_H
#define BOOSTER_PERL_REGEX_H

#include <booster/config.h>
#include <booster/copy_ptr.h>
#include <string>
#include <vector>
#include <booster/backtrace.h>

namespace booster {
	///
	/// \brief Exception that is thrown in case of creation of invalid regex
	/// 
	class regex_error : public booster::runtime_error {
	public:
		regex_error(std::string const &s) : booster::runtime_error(s)
		{
		}
	};

	///
	/// \brief This is a simple wrapper of PCRE library.
	///
	/// It is designed to be used with sub_match, match_results, regex_match and regex_search template functions.
	///
	/// It provides API similar to ones of Boost.Regex but it is also much simplified.
	///
	class BOOSTER_API regex {
	public:
		typedef char value_type;

		explicit regex();
		///
		/// Copy regular expression. Note. This is much more efficient then creating a new expression
		/// with same patter.
		///
		regex(regex const &);
		///
		/// Copy regular expression. Note. This is much more efficient then creating a new expression
		/// with same patter.
		///
		regex const &operator=(regex const &);

		~regex();

		///
		/// Create regular expression using a \a patter and special \a flags. Note, at this point flags
		/// should be normal or perl only (which  are equivalent). May be extended in future.
		///
		/// Throws regex_error in case of invalid expression.
		///
		regex(std::string const &pattern,int flags = normal);


		///
		/// Assigns regular expression using a \a patter and special \a flags. Note, at this point flags
		/// should be normal or perl only (which  are equivalent). May be extended in future.
		///
		/// Throws regex_error in case of invalid expression.
		///
		void assign(std::string const &pattern,int flags = normal);
		///
		/// Get expression flags. Now always 0.
		///
		int flags() const;
		///
		/// Get the string that the regular expression was created with.
		///
		std::string str() const;
		///
		/// Get number of captured subexpressions.
		///
		unsigned mark_count() const;

		///
		/// Match the expression in the text in range [begin,end) exactly. Parameter \a flags currently unused.
		///
		/// Return true if matches
		///

		bool match(char const *begin,char const *end,int flags = 0) const;
		///
		/// Match the expression in the text in range [begin,end) exactly. Parameter \a flags currently unused.
		///
		/// Return true if matches, and stores captured sub-patterns in \a marks. Each pair represents
		/// a text in rage [begin+first,begin+second).
		///
		/// If no such patter was captured, returns (-1,-1) as pair.
		///
		bool match(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int flags = 0) const;

		///
		/// Search the expression in the text in range [begin,end). Parameter \a flags currently unused.
		///
		/// Return true if found.
		///
		
		bool search(char const *begin,char const *end,int flags = 0) const;
		///
		/// Search the expression in the text in range [begin,end). Parameter \a flags currently unused.
		///
		/// Return true if found, and stores captured sub-patterns in \a marks. Each pair represents
		/// a text in rage [begin+first,begin+second).
		///
		/// If no such patter was captured, returns (-1,-1) as pair.
		///
		bool search(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int flags = 0) const;

		///
		/// Returns true if the expression wasn't assigned.
		///
		bool empty() const;

		static const int perl = 0; ///< Constant for expression type - Perl Compatible Regex.
		static const int normal = 0; ///< Constant for expression type - synonym of perl, default.

	private:
		struct data;
		copy_ptr<data> d;
	};
} // booster


#endif
