//
//  Copyright (c) 2010 Artyom Beilis (Tonkikh)
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
#include <stdexcept>

namespace booster {
	class regex_error : public std::runtime_error {
	public:
		regex_error(std::string const &s) : std::runtime_error(s)
		{
		}
	};

	class BOOSTER_API regex {
	public:
		typedef char value_type;

		explicit regex();
		regex(regex const &);
		regex const &operator=(regex const &);
		~regex();

		regex(std::string const &pattern,int flags = normal);

		void assign(std::string const &pattern,int flags = normal);
		int flags() const;
		std::string str() const;
		unsigned mark_count() const;

		bool match(char const *begin,char const *end,int flags = 0) const;
		bool match(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int flags = 0) const;
		
		bool search(char const *begin,char const *end,int flags = 0) const;
		bool search(char const *begin,char const *end,std::vector<std::pair<int,int> > &marks,int flags = 0) const;

		bool empty() const;

		static const int perl = 0;
		static const int normal = 0; 
	private:
		struct data;
		copy_ptr<data> d;
	};
} // booster


#endif
