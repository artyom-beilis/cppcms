//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef CPPCMS_LOCALE_BOUNDARY_HPP_INCLUDED
#define CPPCMS_LOCALE_BOUNDARY_HPP_INCLUDED

#include "defs.h"
#include "config.h"
#include "cstdint.h"

#include <string>
#include <locale>
#include <vector>
#include <iterator>
#include <algorithm>
#include <typeinfo>

namespace cppcms {
    namespace locale {
        
        namespace boundary {
            ///
            /// \brief The enum that describes possible break types
            ///
            typedef enum {
                character,  ///< Find character boundaries
                word,       ///< Find word boundaries
                sentence,   ///< Find sentence boundaries
                line        ///< Find a positions suitable for line breaks
            } boundary_type;

            ///
            /// Flags used with word boundary analysis -- the type of word found
            ///
            typedef enum {
                number  = 1 << 0,   ///< Word that appear to be a number
                letter  = 1 << 1,   ///< Word that contains letters
                kana    = 1 << 2,   ///< Word that contains kana characters
                ideo    = 1 << 3,   ///< Word that contains ideographic characters
            } word_type;
            ///
            /// Flags that describe a type of line break
            ///
            typedef enum {
                soft    = 1 << 0,   ///< Optional line break
                hard    = 1 << 1    ///< Mandatory line break
            } line_break_type;
            
           
            namespace impl {

                ///
                /// \ brief a structure that describes break position
                /// 
                struct break_info {

                    ///
                    /// Default constructor -- all fields are zero
                    /// 
                    break_info() : 
                        offset(0),
                        next(0),
                        prev(0),
                        brk (0),
                        reserved(0)
                    {
                    }
                    ///
                    /// Create a break info with offset \a v
                    ///
                    break_info(unsigned v) :
                        offset(v),
                        next(0),
                        prev(0),
                        brk (0),
                        reserved(0)
                    {
                    }

                    uint32_t offset;            ///< Offset from the beginning of the text where break occurs
                    uint32_t next       : 10;   ///< The flag describing a word following the break position
                    uint32_t prev       : 10;   ///< The flag describing a word preceding the break position
                    uint32_t brk        : 10;   ///< The flag describing a type of line break
                    uint32_t reserved   : 2;
                   
                    ///
                    /// Comparison operator allowing usage of break_info in STL algorithms
                    /// 
                    bool operator<(break_info const &other) const
                    {
                        return offset < other.offset;
                    }
                };
               
                ///
                /// The index of all found boundaries. Note: for a string with length \a len, 0 and len are always considered
                /// boundaries
                /// 
                typedef std::vector<break_info> index_type;


                ///
                /// Find boundary positions of type \a t for text in range [begin,end) using locale \a loc
                ///
                template<typename CharType>
                index_type map(boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc=std::locale());
                
                ///
                /// Find boundary positions of type \a t for string \a str using locale \a loc
                ///
                template<typename CharType>
                static index_type map(
                                boundary_type t,
                                std::basic_string<CharType> const &str,
                                std::locale const &loc=std::locale())
                {
                    return cppcms::locale::boundary::impl::map<CharType>(t,str.data(),str.data()+str.size(),loc);
                }
                
                template<>
                CPPCMS_API index_type 
                map(boundary_type t,char const *begin,char const *end,std::locale const &loc);

                #ifndef CPPCMS_NO_STD_WSTRING
                template<>
                CPPCMS_API index_type 
                map(boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc);
                #endif

                #ifdef CPPCMS_HAS_CHAR16_T
                template<>
                CPPCMS_API index_type 
                map(boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc);
                #endif

                #ifdef CPPCMS_HAS_CHAR32_T
                template<>
                CPPCMS_API index_type 
                map(boundary_type t,char32_t const *begin,char32_t const *end,std::locale const &loc);
                #endif
            } // impl

            namespace details {

                template<typename IteratorType,typename CategoryType = typename std::iterator_traits<IteratorType>::iterator_category>
                struct mapping_traits {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                    static impl::index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        std::basic_string<char_type> str(b,e);
                        return impl::map(t,str,l);
                    }
                };

                template<typename IteratorType>
                struct mapping_traits<IteratorType,std::random_access_iterator_tag> {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;

                    static impl::index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        impl::index_type result;
                        //
                        // Optimize for most common cases
                        //
                        // C++0x requires that string is continious in memory and all string implementations
                        // do this because of c_str() support. 
                        //

                        if( 
                            (
                                typeid(IteratorType) == typeid(typename std::basic_string<char_type>::iterator)
                                || typeid(IteratorType) == typeid(typename std::basic_string<char_type>::const_iterator)
                                || typeid(IteratorType) == typeid(typename std::vector<char_type>::iterator)
                                || typeid(IteratorType) == typeid(typename std::vector<char_type>::const_iterator)
                            )
                            &&
                                b!=e
                          )
                        {
                            char const *begin = &*b;
                            char const *end = begin + (e-b);
                            impl::index_type tmp=impl::map(t,begin,end,l);
                            result.swap(tmp);
                        }
                        else{
                            std::basic_string<char_type> str(b,e);
                            impl::index_type tmp=impl::map(t,str,l);
                            result.swap(tmp);
                        }
                        return result;
                    }
                };

            } // details 

            ///
            /// \brief Class the holds boundary mapping of the text that can be used with iterators
            ///
            /// When the object is created in creates index and provides access to it with iterators.
            /// it is used mostly together with break_iterator and token_iterator
            ///
            /// This class stores iterators to the original text, so you should be careful with iterators
            /// invalidation. The text, iterators pointing to should not change
            ///

            template<class RangeIterator>
            class mapping {
            public:
                typedef RangeIterator iterator;
                typedef typename RangeIterator::base_iterator base_iterator;
                typedef typename std::iterator_traits<base_iterator>::value_type char_type;

                ///
                /// create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc
                ///
                mapping(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,0);
                }

                mapping(boundary_type type,base_iterator begin,base_iterator end,unsigned mask,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,mask);
                }

                ///
                /// create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc
                ///
                void map(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,0);
                }

                void map(boundary_type type,base_iterator begin,base_iterator end,unsigned mask,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,mask);
                }

                mapping()
                {
                    mask_=0;
                }

                template<typename ORangeIterator>
                mapping(mapping<ORangeIterator> const &other) :
                    index_(other.index_),
                    begin_(other.begin_),
                    end_(other.end_),
                    mask_(other.mask_)
                {
                }
                
                template<typename ORangeIterator>
                void swap(mapping<ORangeIterator> &other)
                {
                    index_.swap(other.index_),
                    std::swap(begin_,other.begin_);
                    std::swap(end_,other.end_);
                    std::swap(mask_,other.mask_);
                }

                template<typename ORangeIterator>
                mapping const &operator=(mapping<ORangeIterator> const &other)
                {
                    index_=other.index_;
                    begin_=other.begin_;
                    end_=other.end_;
                    mask_=other.mask_;
                }

                unsigned mask() const
                {
                    return mask_;
                }
                void mask(unsigned u)
                {
                    mask_ = u;
                }

                ///
                /// Get \a begin iterator used when object was created
                ///
                RangeIterator begin()
                {
                    return RangeIterator(*this,true,mask_);
                }
                ///
                /// Get \a end iterator used when object was created
                ///
                RangeIterator end()
                {
                    return RangeIterator(*this,false,mask_);
                }

            private:
                void create_mapping(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc,unsigned mask)
                {
                    impl::index_type idx=details::mapping_traits<base_iterator>::map(type,begin,end,loc);
                    index_.swap(idx);
                    begin_ = begin;
                    end_ = end;
                    mask_=mask;
                }

                template<typename I> 
                friend class break_iterator; 
                template<typename I,typename V>
                friend class token_iterator; 
                template<typename I>
                friend class mapping;

                base_iterator begin_,end_;
                impl::index_type index_;
                unsigned mask_;
            };

            ///
            /// \brief break_iterator is bidirectional iterator that returns text boundary positions
            ///
            /// This iterator is used when boundary points are more interesting then text chunks themselves.
            ///
            template<typename IteratorType>
            class break_iterator : public std::iterator<std::bidirectional_iterator_tag,IteratorType> {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                typedef IteratorType base_iterator;
                
                break_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0)
                {
                }

                break_iterator(break_iterator<IteratorType> const &other):
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_)
                {
                }
                
                break_iterator const &operator=(break_iterator<IteratorType> const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_=other.mask_;
                    }
                    return *this;
                }


                ///
                /// Set break iterator to new position \a p. 
                ///
                /// \a p should be in range of [\a begin, \a end] of the original text.
                ///
                /// If \a p points to actual boundary point, break iterator would point to this point, otherwise,
                /// it would point to the proceeding boundary point.
                ///
                /// Note: break iterator should be created and mapping should be assigned to it.
                ///
                /// \code 
                ///    break_iterator<char const *>  begin(map),end;
                ///    begin=some_point_in_text; // Ok
                ///    end=some_point_in_text; // Wrong!
                /// \endcode
                ///
                break_iterator const &operator=(IteratorType p)
                {
                    at_most(p);
                    return *this;
                }

                ///
                /// Create a break iterator pointing to the beginning of the range.
                ///
                /// \a map -- is the mapping used with iterator. Should be valid all the time iterator is used.
                ///
                /// \a mask -- mask of flags for which boundary points are counted. If \a mask is 0, all boundary
                /// points are used, otherwise, only points giving break_info::brk & mask !=0 are used, others are skipped.
                ///
                /// For example.
                ///
                /// \code
                ///   boundary::mapping<char *> a_map(boundary::line,begin,end);
                ///   boundart::break_iterator<char *> p(a_map,boundary::soft),e;
                //// \endcode
                /// 
                /// Would create an iterator p that would iterate only over soft line break points.
                ///
                break_iterator(mapping<break_iterator> const &map,bool begin,unsigned mask) :
                    map_(&map),
                    mask_(mask)
                {
                    if(begin)
                        offset_=0;
                    else
                        offset_=map_->index_.size();
                }

                ///
                /// Compare two iterators,              
                ///
                
                bool operator==(break_iterator<IteratorType> const &other) const
                {
                    return  (map_ == other.map_ && offset_==other.offset_);
                }

                ///
                /// Opposite of ==
                ///
                bool operator!=(break_iterator<IteratorType> const &other) const
                {
                    return !(*this==other);
                }

                ///
                /// Return the underlying iterator to the boundary point.
                ///
                IteratorType operator*() const
                {
                    return map_->begin_ + map_->index_[offset_].offset;
                }

                ///
                /// return flags describing the break point, like \a line_break_type
                ///
                
                unsigned flag() const
                {
                    return map_->index_[offset_].brk;
                }
                
                break_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                break_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                break_iterator operator++(int unused) 
                {
                    break_iterator tmp(*this);
                    next();
                    return tmp;
                }

                break_iterator operator--(int unused) 
                {
                    break_iterator tmp(*this);
                    prev();
                    return tmp;
                }


            private:
                void at_most(IteratorType p)
                {
                    unsigned diff =  p - map_->begin_;
                    impl::index_type::iterator ptr = std::lower_bound(map_->index_.begin(),map_->index_.end(),impl::break_info(diff));
                    if(ptr==map_->index_.end())
                        offset_=map_->index_.size()-1;
                    else
                        offset_=ptr - map_->index_.begin();
                    if(mask_==0)
                        return;
                    while(offset_ > 0 && (map_->index_[offset_].brk & mask_) == 0)
                        offset_--;
                }
                bool at_end() const
                {
                    return !map_ || offset_ >= map_->index_.size();
                }
                void next()
                {
                    do {
                        offset_++;
                    }while(!at_end() && mask_ != 0 && ((map_->index_[offset_].brk & mask_) == 0));
                }
                void prev()
                {
                    do {
                        offset_--;
                    }while(offset_ >0 && mask_ != 0 && ((map_->index_[offset_].brk & mask_) == 0));
                }

                mapping<break_iterator> const * map_;
                size_t offset_;
                unsigned mask_;
            };

            ///
            /// \brief token iterator is an iterator that returns text chunks between boundary positions
            ///
            /// It is similar to break iterator, but it rather "points" to string then iterator. It is used when we are more
            /// interested in text chunks themselves then boundary points.
            ///
            
            template<typename IteratorType,typename ValueType = std::basic_string<typename std::iterator_traits<IteratorType>::value_type> >
            class token_iterator : public std::iterator<std::bidirectional_iterator_tag,ValueType> {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                typedef IteratorType base_iterator;
                                
                token_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0)
                {
                }

                token_iterator(token_iterator const &other):
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_)
                {
                }
                
                token_iterator const &operator=(token_iterator<IteratorType,ValueType> const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_=other.mask_;
                    }
                    return *this;
                }

                token_iterator const &operator=(IteratorType p)
                {
                    at_most(p);
                    return *this;
                }
                token_iterator(mapping<token_iterator> const &map,bool begin,unsigned mask) :
                    map_(&map),
                    mask_(mask)
                {
                    if(begin)
                        offset_ = 0;
                    else
                        offset_=map_->index_.size()-1;
                    if(begin && mask_!=0 && (map_->index_[0].next & mask_)==0)
                        next();
                }

                bool operator==(token_iterator<IteratorType,ValueType> const &other) const
                {
                    return  (map_ == other.map_ && offset_==other.offset_);
                }

                bool operator!=(token_iterator<IteratorType,ValueType> const &other) const
                {
                    return !(*this==other);
                }

                ValueType operator*() const
                {
                    IteratorType ob=map_->begin_ + map_->index_[offset_].offset;
                    IteratorType oe=map_->begin_ + map_->index_[offset_+1].offset;
                    return ValueType(ob,oe);
                }
                ///
                /// return flags describing the selected string, like \a word_type
                ///
                
                unsigned flag() const
                {
                    return map_->index_[offset_].next;
                }
                
                token_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                token_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                token_iterator operator++(int unused) 
                {
                    token_iterator<IteratorType,ValueType> tmp(*this);
                    next();
                    return tmp;
                }

                token_iterator operator--(int unused) 
                {
                    token_iterator<IteratorType,ValueType> tmp(*this);
                    prev();
                    return tmp;
                }


            private:
                void at_most(IteratorType p)
                {
                    unsigned diff =  p - map_->begin_;
                    impl::index_type::iterator ptr = std::lower_bound(map_->index_.begin(),map_->index_.end(),impl::break_info(diff));
                    if(ptr==map_->index_.end())
                        offset_=map_->index_.size()-1;
                    else
                        offset_=ptr - map_->index_.begin();
                    if(mask_==0)
                        return;
                    while(offset_ > 0 && (map_->index_[offset_].next & mask_) == 0)
                        offset_--;
                    if(offset_==0 && (map_->index_[offset_].next & mask_) == 0)
                        next();
                }
                bool at_end() const
                {
                    return !map_ || offset_ >= map_->index_.size()-1;
                }
                void next()
                {
                    do {
                        offset_++;
                    }while(!at_end() && mask_ != 0 && ((map_->index_[offset_].next & mask_) == 0));
                }
                void prev()
                {
                    do {
                        offset_--;
                    }while(offset_ >0 && mask_ != 0 && ((map_->index_[offset_].next & mask_) == 0));
                }

                mapping<token_iterator> const * map_;
                size_t offset_;
                unsigned mask_;
            };
            
            
        } // boundary
    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
