//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_BOUNDARY_BOUND_H_INCLUDED
#define BOOSTER_LOCALE_BOUNDARY_BOUND_H_INCLUDED

#include <booster/locale/boundary/types.h>

namespace booster {
namespace locale {
namespace boundary {

    ///
    /// \addtogroup boundary
    /// @{

    ///
    /// \brief This class represents a boundary point in the text. 
    ///
    /// It represents a pair - an iterator and a rule that defines this 
    /// point.
    ///
    template<typename IteratorType>
    class bound  {
    public:
        typedef IteratorType iterator_type;

        ///
        /// Empty default constructor
        ///
        bound() : rule_(0) {}
        
        ///
        /// Create a new bound using iterator \p and a rule \a r
        ///
        bound(iterator_type p,rule_type r) :
            iterator_(p),
            rule_(r)
        {
        }
        ///
        /// Set an new iterator value \a i
        ///
        void iterator(iterator_type i)
        {
            iterator_ = i;
        }
        ///
        /// Set an new rule value \a r
        ///
        void rule(rule_type r)
        {
            rule_ = r;
        }
        ///
        /// Fetch an iterator
        ///
        iterator_type iterator() const 
        {
            return iterator_;
        }
        ///
        /// Fetch a rule
        ///
        rule_type rule() const
        {
            return rule_;
        }
        ///
        /// Check if two boundary points are the same
        ///
        bool operator==(bound const &other) const
        {
            return iterator_ == other.iterator_ && rule_ = other.rule_;
        }
        ///
        /// Check if two boundary points are different
        ///
        bool operator!=(bound const &other) const
        {
            return !(*this==other);
        }
        ///
        /// Check if the boundary point points to same location as an iterator \a other
        ///
        bool operator==(iterator_type const &other) const
        {
            return iterator_ == other;
        }
        ///
        /// Check if the boundary point points to different location from an iterator \a other
        ///
        bool operator!=(iterator_type const &other) const
        {
            return iterator_ != other;
        }

        ///
        /// Automatic cast to the iterator it represents
        ///
        operator iterator_type ()const
        {
            return iterator_;
        }

    private:
        iterator_type iterator_;
        rule_type rule_;
       
    };
    ///
    /// Check if the boundary point \a r points to same location as an iterator \a l
    ///
    template<typename BaseIterator>
    bool operator==(BaseIterator const &l,bound<BaseIterator> const &r)
    {
        return r==l;
    }
    ///
    /// Check if the boundary point \a r points to different location from an iterator \a l
    ///
    template<typename BaseIterator>
    bool operator!=(BaseIterator const &l,bound<BaseIterator> const &r)
    {
        return r!=l;
    }

    /// @}
    

} // boundary
} // locale
} // boost


#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
