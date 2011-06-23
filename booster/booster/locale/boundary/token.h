//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_BOUNDARY_TOKEN_H_INCLUDED
#define BOOSTER_LOCALE_BOUNDARY_TOKEN_H_INCLUDED
#include <booster/config.h>
#ifdef BOOSTER_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <locale>
#include <string>
#include <iosfwd>
#include <iterator>


namespace booster {
namespace locale {
namespace boundary {

    ///
    /// \addtogroup boundary
    /// @{

    ///
    /// \brief a token object that represents a pair of two iterators that define the range where
    /// this token exits and a rule that defines it.
    ///
    template<typename IteratorType>
    class token : public std::pair<IteratorType,IteratorType> {
    public:
        ///
        /// The type of the underlying character 
        ///
        typedef typename std::iterator_traits<IteratorType>::value_type char_type;
        ///
        /// The type of the string it is converted to
        ///
        typedef std::basic_string<char_type> string_type;
        ///
        /// The value that iterators return  - the character itself
        ///
        typedef char_type value_type;
        ///
        /// The iterator that allows to iterate the range
        ///
        typedef IteratorType iterator;
        ///
        /// The iterator that allows to iterate the range
        ///
        typedef IteratorType const_iterator;
        ///
        /// The type that represent a difference between two iterators
        ///
        typedef typename std::iterator_traits<IteratorType>::difference_type difference_type;

        ///
        /// Default constructor
        ///
        token() {}
        ///
        /// Create a token using two iterators and a rule that represents this point
        ///
        token(iterator b,iterator e,rule_type r) :
            std::pair<IteratorType,IteratorType>(b,e),
            rule_(r)
        {
        }
        ///
        /// Set the start of the range
        ///
        void begin(iterator const &v)
        {
            this->first = v;
        }
        ///
        /// Set the end of the range
        ///
         void end(iterator const &v)
        {
            this->second = v;
        }

        ///
        /// Get the start of the range
        ///
        IteratorType begin() const 
        {
            return this->first;
        }
        ///
        /// Set the end of the range
        ///
        IteratorType end() const
        {
            return this->second;
        }

        ///
        /// Convert the range to a string automatically
        ///
        template <class T, class A>
        operator std::basic_string<char_type, T, A> ()const
        {
            return std::basic_string<char_type, T, A>(this->first, this->second);
        }
        
        ///
        /// Create a string from the range explicitly
        ///
        string_type str() const
        {
            return string_type(begin(),end());
        }

        ///
        /// Get the length of the text chunk
        ///

        size_t length() const
        {
            return std::distance(begin(),end());
        }

        ///
        /// Check if the token is empty
        ///
        bool empty() const
        {
            return begin() == end();
        }

        ///
        /// Get the rule that is used for selection of this token.
        ///
        rule_type rule() const
        {
            return rule_;
        }
        ///
        /// Set a rule that is used for token selection
        ///
        void rule(rule_type r)
        {
            rule_ = r;
        }
    private:
        rule_type rule_;
       
    };
    
    
    ///
    /// Write the token to the stream character by character
    ///
    template<typename CharType,typename TraitsType,typename Iterator>
    std::basic_ostream<CharType,TraitsType> &operator<<(
            std::basic_ostream<CharType,TraitsType> &out,
            token<Iterator> const &tok)
    {
        for(Iterator p=tok.begin(),e=tok.end();p!=e;++p)
            out << *p;
        return out;
    }

    /// @}

} // boundary
} // locale
} // boost

#ifdef BOOSTER_MSVC
#pragma warning(pop)
#endif

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
