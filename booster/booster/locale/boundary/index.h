//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOSTER_LOCALE_BOUNDARY_INDEX_H_INCLUDED
#define BOOSTER_LOCALE_BOUNDARY_INDEX_H_INCLUDED

#include <booster/config.h>
#include <booster/locale/boundary/types.h>
#include <booster/locale/boundary/facets.h>
#include <booster/locale/boundary/token.h>
#include <booster/iterator/iterator_facade.h>
#include <booster/shared_ptr.h>
#include <booster/cstdint.h>
#include <booster/assert.h>
#ifdef BOOSTER_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <string>
#include <locale>
#include <vector>
#include <iterator>
#include <algorithm>
#include <booster/backtrace.h>

#include <iostream>

namespace booster {

    namespace locale {
        
        ///
        /// \brief This namespae contains all operations required for boundary analysis of text
        ///
        namespace boundary {
            ///
            /// \defgroup boundary Boundary Analysis
            ///
            /// This module contains all operations required for boundary analysis of text: character, word, like and sentence boundaries
            ///
            /// @{
            ///

            /// \cond INTERNAL

            namespace details {

                template<typename IteratorType,typename CategoryType = typename std::iterator_traits<IteratorType>::iterator_category>
                struct mapping_traits {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                    static index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        std::basic_string<char_type> str(b,e);
                        return std::use_facet<boundary_indexing<char_type> >(l).map(t,str.c_str(),str.c_str()+str.size());
                    }
                };

                template<typename CharType,typename SomeIteratorType>
                struct linear_iterator_traits {
                    static const bool is_linear = false;
                };

                template<typename CharType>
                struct linear_iterator_traits<CharType,typename std::basic_string<CharType>::iterator> {
                    static const bool is_linear = true;
                };

                template<typename CharType>
                struct linear_iterator_traits<CharType,typename std::basic_string<CharType>::const_iterator> {
                    static const bool is_linear = true;
                };
                
                template<typename CharType>
                struct linear_iterator_traits<CharType,typename std::vector<CharType>::iterator> {
                    static const bool is_linear = true;
                };

                template<typename CharType>
                struct linear_iterator_traits<CharType,typename std::vector<CharType>::const_iterator> {
                    static const bool is_linear = true;
                };

                template<typename CharType>
                struct linear_iterator_traits<CharType,CharType *> {
                    static const bool is_linear = true;
                };

                template<typename CharType>
                struct linear_iterator_traits<CharType,CharType const *> {
                    static const bool is_linear = true;
                };


                template<typename IteratorType>
                struct mapping_traits<IteratorType,std::random_access_iterator_tag> {

                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;



                    static index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        index_type result;

                        //
                        // Optimize for most common cases
                        //
                        // C++0x requires that string is continious in memory and all known
                        // string implementations
                        // do this because of c_str() support. 
                        //

                        if(linear_iterator_traits<char_type,IteratorType>::is_linear && b!=e)
                        {
                            char_type const *begin = &*b;
                            char_type const *end = begin + (e-b);
                            index_type tmp=std::use_facet<boundary_indexing<char_type> >(l).map(t,begin,end);
                            result.swap(tmp);
                        }
                        else {
                            std::basic_string<char_type> str(b,e);
                            index_type tmp = std::use_facet<boundary_indexing<char_type> >(l).map(t,str.c_str(),str.c_str()+str.size());
                            result.swap(tmp);
                        }
                        return result;
                    }
                };

                template<typename BaseIterator>
                class mapping {
                public:
                    typedef BaseIterator base_iterator;
                    typedef typename std::iterator_traits<base_iterator>::value_type char_type;


                    mapping(boundary_type type,
                            base_iterator begin,
                            base_iterator end,
                            std::locale const &loc) 
                        :   
                            index_(new index_type()),
                            begin_(begin),
                            end_(end)
                    {
                        index_type idx=details::mapping_traits<base_iterator>::map(type,begin,end,loc);
                        index_->swap(idx);
                    }

                    mapping()
                    {
                    }

                    index_type const &index() const
                    {
                        return *index_;
                    }

                    base_iterator begin() const
                    {
                        return begin_;
                    }

                    base_iterator end() const
                    {
                        return end_;
                    }

                private:
                    booster::shared_ptr<index_type> index_;
                    base_iterator begin_,end_;
                };

                template<typename BaseIterator>
                class token_index_iterator : 
                    public booster::iterator_facade<
                        token_index_iterator<BaseIterator>,
                        token<BaseIterator>,
                        booster::bidirectional_traversal_tag,
                        token<BaseIterator> const &
                    >
                {
                public:
                    typedef BaseIterator base_iterator;
                    typedef mapping<base_iterator> mapping_type;
                    typedef token<base_iterator> token_type;
                    
                    token_index_iterator() : current_(0,0),map_(0)
                    {
                    }

                    token_index_iterator(base_iterator p,mapping_type const *map,rule_type mask,bool full_select) :
                        map_(map),
                        mask_(mask),
                        full_select_(full_select)
                    {
                        set(p);
                    }
                    token_index_iterator(bool is_begin,mapping_type const *map,rule_type mask,bool full_select) :
                        map_(map),
                        mask_(mask),
                        full_select_(full_select)
                    {
                        if(is_begin)
                            set_begin();
                        else
                            set_end();
                    }

                    token_type const &dereference() const
                    {
                        return value_;
                    }

                    bool equal(token_index_iterator const &other) const
                    {
                        return map_ == other.map_ && current_.second == other.current_.second;
                    }

                    void increment()
                    {
                        std::pair<size_t,size_t> next = current_;
                        if(full_select_) {
                            next.first = next.second;
                            while(next.second < size()) {
                                next.second++;
                                if(valid_offset(next.second))
                                    break;
                            }
                            if(next.second == size())
                                next.first = next.second - 1;
                        }
                        else {
                            while(next.second < size()) {
                                next.first = next.second;
                                next.second++;
                                if(valid_offset(next.second))
                                    break;
                            }
                        }
                        update_current(next);
                    }

                    void decrement()
                    {
                        std::pair<size_t,size_t> next = current_;
                        if(full_select_) {
                            while(next.second >1) {
                                next.second--;
                                if(valid_offset(next.second))
                                    break;
                            }
                            next.first = next.second;
                            while(next.first >0) {
                                next.first--;
                                if(valid_offset(next.first))
                                    break;
                            }
                        }
                        else {
                            while(next.second >1) {
                                next.second--;
                                if(valid_offset(next.second))
                                    break;
                            }
                            next.first = next.second - 1;
                        }
                        update_current(next);
                    }

                private:

                    void set_end()
                    {
                        current_.first  = size() - 1;
                        current_.second = size();
                        value_ = token_type(map_->end(),map_->end(),0);
                    }
                    void set_begin()
                    {
                        current_.first = current_.second = 0;
                        value_ = token_type(map_->begin(),map_->begin(),0);
                        increment();
                    }

                    void set(base_iterator p)
                    {
                        size_t dist=std::distance(map_->begin(),p);
                        index_type::const_iterator b=map_->index().begin(),e=map_->index().end();
                        index_type::const_iterator 
                            bound=std::upper_bound(b,e,break_info(dist));
                        while(bound != e && (bound->rule & mask_)==0)
                            bound++;

                        current_.first = current_.second = bound - b;
                        
                        if(full_select_) {
                            while(current_.first > 0) {
                                current_.first --;
                                if(valid_offset(current_.first))
                                    break;
                            }
                        }
                        else {
                            if(current_.first > 0)
                                current_.first --;
                        }
                        value_.first = map_->begin();
                        std::advance(value_.first,get_offset(current_.first));
                        value_.second = value_.first;
                        std::advance(value_.second,get_offset(current_.second) - get_offset(current_.first));

                        update_rule();
                    }

                    void update_current(std::pair<size_t,size_t> pos)
                    {
                        std::ptrdiff_t first_diff = get_offset(pos.first) - get_offset(current_.first);
                        std::ptrdiff_t second_diff = get_offset(pos.second) - get_offset(current_.second);
                        std::advance(value_.first,first_diff);
                        std::advance(value_.second,second_diff);
                        current_ = pos;
                        update_rule();
                    }

                    void update_rule()
                    {
                        if(current_.second != size()) {
                            value_.rule(index()[current_.second].rule);
                        }
                    }
                    size_t get_offset(size_t ind) const
                    {
                        if(ind == size())
                            return index().back().offset;
                        return index()[ind].offset;
                    }

                    bool valid_offset(size_t offset) const
                    {
                        return  offset == 0 
                                || offset == size() // make sure we not acess index[size]
                                || (index()[offset].rule & mask_)!=0;
                    }
                    
                    size_t size() const
                    {
                        return index().size();
                    }
                    
                    index_type const &index() const
                    {
                        return map_->index();
                    }
                
                    
                    token_type value_;
                    std::pair<size_t,size_t> current_;
                    mapping_type const *map_;
                    rule_type mask_;
                    bool full_select_;
                };
                            
                template<typename BaseIterator>
                class bound_index_iterator : 
                    public booster::iterator_facade<
                        bound_index_iterator<BaseIterator>,
                        bound<BaseIterator>,
                        booster::bidirectional_traversal_tag,
                        bound<BaseIterator> const &
                    >
                {
                public:
                    typedef BaseIterator base_iterator;
                    typedef mapping<base_iterator> mapping_type;
                    typedef bound<base_iterator> bound_type;
                    
                    bound_index_iterator() : current_(0),map_(0)
                    {
                    }

                    bound_index_iterator(bool is_begin,mapping_type const *map,rule_type mask) :
                        map_(map),
                        mask_(mask)
                    {
                        if(is_begin)
                            set_begin();
                        else
                            set_end();
                    }
                    bound_index_iterator(base_iterator p,mapping_type const *map,rule_type mask) :
                        map_(map),
                        mask_(mask)
                    {
                        set(p);
                    }

                    bound_type const &dereference() const
                    {
                        return value_;
                    }

                    bool equal(bound_index_iterator const &other) const
                    {
                        return map_ == other.map_ && current_ == other.current_;
                    }

                    void increment()
                    {
                        size_t next = current_;
                        while(next < size()) {
                            next++;
                            if(valid_offset(next))
                                break;
                        }
                        update_current(next);
                    }

                    void decrement()
                    {
                        size_t next = current_;
                        while(next>0) {
                            next--;
                            if(valid_offset(next))
                                break;
                        }
                        update_current(next);
                    }

                private:
                    void set_end()
                    {
                        current_ = size();
                        value_ = bound_type(map_->end(),0);
                    }
                    void set_begin()
                    {
                        current_ = 0;
                        value_ = bound_type(map_->begin(),0);
                    }

                    void set(base_iterator p)
                    {
                        size_t dist =  std::distance(map_->begin(),p);

                        index_type::const_iterator b=index().begin();
                        index_type::const_iterator e=index().end();
                        index_type::const_iterator ptr = std::lower_bound(b,e,break_info(dist));

                        if(ptr==index().end())
                            current_=size()-1;
                        else
                            current_=ptr - index().begin();

                        while(!valid_offset(current_))
                            current_ ++;

                        std::ptrdiff_t diff = get_offset(current_) - dist;
                        std::advance(p,diff);
                        value_.iterator(p);
                        update_rule();
                    }

                    void update_current(size_t pos)
                    {
                        std::ptrdiff_t diff = get_offset(pos) - get_offset(current_);
                        base_iterator i=value_.iterator();
                        std::advance(i,diff);
                        current_ = pos;
                        value_.iterator(i);
                        update_rule();
                    }

                    void update_rule()
                    {
                        if(current_ != size()) {
                            value_.rule(index()[current_].rule);
                        }
                    }
                    size_t get_offset(size_t ind) const
                    {
                        if(ind == size())
                            return index().back().offset;
                        return index()[ind].offset;
                    }

                    bool valid_offset(size_t offset) const
                    {
                        return  offset == 0 
                                || offset + 1 >= size() // last and first are always valid regardless of mark
                                || (index()[offset].rule & mask_)!=0;
                    }
                    
                    size_t size() const
                    {
                        return index().size();
                    }
                    
                    index_type const &index() const
                    {
                        return map_->index();
                    }
                
                    
                    bound_type value_;
                    size_t current_;
                    mapping_type const *map_;
                    rule_type mask_;
                };


            } // details

            template<typename BaseIterator>
            class token_index;

            template<typename BaseIterator>
            class bound_index;
            

            ///
            /// \brief This class holds an index of tokens in the text range and allows to iterate over them 
            ///
            /// When the object is created it creates index and provides access to it with iterators.
            ///
            /// It is used mostly with break_iterator and token_iterator. For each boundary point it
            /// provides the description mark that allows distinguishing between different types of boundaries.
            /// For example, it marks whether a sentence terminates because a mark like "?" or "." was found or because
            /// a new line symbol is present in the text.
            ///
            /// These marks can be read out with the token_iterator::mark() and break_iterator::mark() member functions.
            ///
            /// This class stores iterators to the original text, so you should be careful about iterator
            /// invalidation. If the iterators on the original text are invalid, you can't use this mapping any more.
            ///

            template<typename BaseIterator>
            class token_index {
            public:
                typedef BaseIterator base_iterator;
                typedef details::token_index_iterator<base_iterator> iterator;
                typedef details::token_index_iterator<base_iterator> const_iterator;
                typedef token<base_iterator> value_type;

                token_index() : mask_(0xFFFFFFFFu),full_select_(false)
                {
                }
                token_index(boundary_type type,
                            base_iterator begin,
                            base_iterator end,
                            rule_type mask,
                            std::locale const &loc=std::locale()) 
                    :
                        map_(type,begin,end,loc),
                        mask_(mask),
                        full_select_(false)
                {
                }
                token_index(boundary_type type,
                            base_iterator begin,
                            base_iterator end,
                            std::locale const &loc=std::locale()) 
                    :
                        map_(type,begin,end,loc),
                        mask_(0xFFFFFFFFu),
                        full_select_(false)
                {
                }

                token_index(bound_index<base_iterator> const &);
                token_index const &operator = (bound_index<base_iterator> const &);

                void map(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc=std::locale())
                {
                    map_ = mapping_type(type,begin,end,loc);
                }

                iterator begin() const
                {
                    return iterator(true,&map_,mask_,full_select_);
                }

                iterator end() const
                {
                    return iterator(false,&map_,mask_,full_select_);
                }

                iterator find(base_iterator p) const
                {
                    return iterator(p,&map_,mask_,full_select_);
                }
                
                rule_type rule() const
                {
                    return mask_;
                }
                void rule(rule_type v)
                {
                    mask_ = v;
                }

                bool full_select()  const 
                {
                    return full_select_;
                }

                void full_select(bool v) 
                {
                    full_select_ = v;
                }
                
            private:
                friend class bound_index<base_iterator>;
                typedef details::mapping<base_iterator> mapping_type;
                mapping_type  map_;
                rule_type mask_;
                bool full_select_;
            };

            template<typename BaseIterator>
            class bound_index {
            public:
                typedef BaseIterator base_iterator;
                typedef details::bound_index_iterator<base_iterator> iterator;
                typedef details::bound_index_iterator<base_iterator> const_iterator;
                typedef bound<base_iterator> value_type;
                
                bound_index() : mask_(0xFFFFFFFFu)
                {
                }
                
                bound_index(boundary_type type,
                            base_iterator begin,
                            base_iterator end,
                            rule_type mask,
                            std::locale const &loc=std::locale()) 
                    :
                        map_(type,begin,end,loc),
                        mask_(mask)
                {
                }
                bound_index(boundary_type type,
                            base_iterator begin,
                            base_iterator end,
                            std::locale const &loc=std::locale()) 
                    :
                        map_(type,begin,end,loc),
                        mask_(0xFFFFFFFFu)
                {
                }

                bound_index(token_index<base_iterator> const &other);
                bound_index const &operator=(token_index<base_iterator> const &other);

                void map(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc=std::locale())
                {
                    map_ = mapping_type(type,begin,end,loc);
                }

                iterator begin() const
                {
                    return iterator(true,&map_,mask_);
                }

                iterator end() const
                {
                    return iterator(false,&map_,mask_);
                }

                iterator find(base_iterator p) const
                {
                    return iterator(p,&map_,mask_);
                }
                
                rule_type rule() const
                {
                    return mask_;
                }
                void rule(rule_type v)
                {
                    mask_ = v;
                }

            private:

                friend class token_index<base_iterator>;
                typedef details::mapping<base_iterator> mapping_type;
                mapping_type  map_;
                rule_type mask_;
            };
            
            template<typename BaseIterator>
            token_index<BaseIterator>::token_index(bound_index<BaseIterator> const &other) :
                map_(other.map_),
                mask_(0xFFFFFFFFu),
                full_select_(false)
            {
            }
            
            template<typename BaseIterator>
            bound_index<BaseIterator>::bound_index(token_index<BaseIterator> const &other) :
                map_(other.map_),
                mask_(0xFFFFFFFFu)
            {
            }

            template<typename BaseIterator>
            token_index<BaseIterator> const &token_index<BaseIterator>::operator=(bound_index<BaseIterator> const &other)
            {
                map_ = other.map_;
                return *this;
            }
            
            template<typename BaseIterator>
            bound_index<BaseIterator> const &bound_index<BaseIterator>::operator=(token_index<BaseIterator> const &other)
            {
                map_ = other.map_;
                return *this;
            }
            
            typedef token_index<std::string::const_iterator> stoken_index;
            typedef token_index<std::wstring::const_iterator> wstoken_index;
            #ifdef BOOSTER_HAS_CHAR16_T
            typedef token_index<std::basic_string<char16_t>::const_iterator> u16stoken_index;
            #endif
            #ifdef BOOSTER_HAS_CHAR32_T
            typedef token_index<std::basic_string<char32_t>::const_iterator> u32stoken_index;
            #endif
           
            typedef token_index<char const *> ctoken_index;
            typedef token_index<wchar_t const *> wctoken_index;
            #ifdef BOOSTER_HAS_CHAR16_T
            typedef token_index<char16_t const *> u16ctoken_index;
            #endif
            #ifdef BOOSTER_HAS_CHAR32_T
            typedef token_index<char32_t const *> u32ctoken_index;
            #endif

            typedef bound_index<std::string::const_iterator> sbound_index;
            typedef bound_index<std::wstring::const_iterator> wsbound_index;
            #ifdef BOOSTER_HAS_CHAR16_T
            typedef bound_index<std::basic_string<char16_t>::const_iterator> u16sbound_index;
            #endif
            #ifdef BOOSTER_HAS_CHAR32_T
            typedef bound_index<std::basic_string<char32_t>::const_iterator> u32sbound_index;
            #endif
           
            typedef bound_index<char const *> cbound_index;
            typedef bound_index<wchar_t const *> wcbound_index;
            #ifdef BOOSTER_HAS_CHAR16_T
            typedef bound_index<char16_t const *> u16cbound_index;
            #endif
            #ifdef BOOSTER_HAS_CHAR32_T
            typedef bound_index<char32_t const *> u32cbound_index;
            #endif



        } // boundary

    } // locale
} // boost

///
/// \example boundary.cpp
/// Example of using boundary iterator
/// \example wboundary.cpp
/// Example of using boundary iterator over wide strings
///

#ifdef BOOSTER_MSVC
#pragma warning(pop)
#endif

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
