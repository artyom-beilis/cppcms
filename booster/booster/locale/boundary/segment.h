//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_BOUNDARY_SEGMENT_H_INCLUDED
#define BOOSTER_LOCALE_BOUNDARY_SEGMENT_H_INCLUDED
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
    /// \brief a segment object that represents a pair of two iterators that define the range where
    /// this segment exits and a rule that defines it.
    ///
    /// This type of object is dereferenced by the iterators of segment_index. Using a rule() member function
    /// you can get a specific rule this segment was selected with. For example, when you use
    /// word boundary analysis, you can check if the specific word contains Kana letters by checking (rule() & \ref word_kana)!=0
    /// For a sentence analysis you can check if the sentence is selected because a sentence terminator is found (\ref sentence_term) or
    /// there is a line break (\ref sentence_sep).
    ///
    /// This object can be automatically converted to std::basic_string with the same type of character. It is also
    /// valid range that has begin() and end() member functions returning iterators on the location of the segment.
    ///
    /// \see
    ///
    /// - \ref segment_index
    /// - \ref boundary_point 
    /// - \ref boundary_point_index 
    ///
    template<typename IteratorType>
    class segment : public std::pair<IteratorType,IteratorType> {
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
        segment() {}
        ///
        /// Create a segment using two iterators and a rule that represents this point
        ///
        segment(iterator b,iterator e,rule_type r) :
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
        /// Check if the segment is empty
        ///
        bool empty() const
        {
            return begin() == end();
        }

        ///
        /// Get the rule that is used for selection of this segment.
        ///
        rule_type rule() const
        {
            return rule_;
        }
        ///
        /// Set a rule that is used for segment selection
        ///
        void rule(rule_type r)
        {
            rule_ = r;
        }
    private:
        rule_type rule_;
       
    };
    
    
    ///
    /// Write the segment to the stream character by character
    ///
    template<typename CharType,typename TraitsType,typename Iterator>
    std::basic_ostream<CharType,TraitsType> &operator<<(
            std::basic_ostream<CharType,TraitsType> &out,
            segment<Iterator> const &tok)
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
