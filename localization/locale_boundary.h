//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
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
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4275 4251 4231 4660)
#endif
#include <string>
#include <locale>
#include <vector>
#include <iterator>
#include <algorithm>
#include <typeinfo>
#include <iterator>
#include <stdexcept>




namespace cppcms {

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

            ///
            /// The enum that describes possible break types
            ///
            typedef enum {
                character,  ///< Find character boundaries
                word,       ///< Find word boundaries
                sentence,   ///< Find sentence boundaries
                line        ///< Find a positions suitable for line breaks
            } boundary_type;

            ///
            /// Flags used with word boundary analysis -- the type of the word found
            ///
            typedef enum {
                word_none       =  0x0000F,   ///< Not a word
                word_number     =  0x000F0,   ///< Word that appear to be a number
                word_letter     =  0x00F00,   ///< Word that contains letters, excluding kana and ideographic characters 
                word_kana       =  0x0F000,   ///< Word that contains kana characters
                word_ideo       =  0xF0000,   ///< Word that contains ideographic characters
                word_any        =  0xFFFF0,   ///< Any word including numbers, 0 is special flag, equivalent to 15
                word_letters    =  0xFFF00,   ///< Any word, excluding numbers but including letters, kana and ideograms.
                word_kana_ideo  =  0xFF000,   ///< Word that includes kana or ideographic characters
                word_mask       =  0xFFFFF    ///< Maximal used mask
            } word_type;
            ///
            /// Flags that describe a type of line break
            ///
            typedef enum {
                line_soft       =  0x0F,   ///< Soft line break: optional but not required
                line_hard       =  0xF0,   ///< Hard line break: like break is required (as per CR/LF)
                line_any        =  0xFF,   ///< Soft or Hard line break
                line_mask       =  0xFF
            } line_break_type;
            
            ///
            /// Flags that describe a type of sentence break
            ///
            typedef enum {
                sentence_term   =  0x0F,    ///< The sentence was terminated with a sentence terminator 
                                            ///  like ".", "!" possible followed by hard separator like CR, LF, PS
                sentence_sep    =  0xF0,    ///< The sentence does not contain terminator like ".", "!" but ended with hard separator
                                            ///  like CR, LF, PS or end of input.
                sentence_any    =  0xFF,    ///< Either first or second sentence break type;.
                sentence_mask   =  0xFF
            } sentence_break_type;

            ///
            /// Flags that describe a type of character break. At this point break iterator does not distinguish different
            /// kinds of characters so it is used for consistency.
            ///
            typedef enum {
                character_any   =  0xF,     ///< Not in use, just for consistency
                character_mask  =  0xF,
            } character_break_type;

            ///
            /// This function returns the mask that covers all variants for specific boundary type
            ///
            inline unsigned boundary_mask(boundary_type t)
            {
                switch(t) {
                case character: return character_mask;
                case word:      return word_mask;
                case sentence:  return sentence_mask;
                case line:      return line_mask;
                default:        return 0;
                }
            }
            
            /// \cond INTERNAL 
            namespace impl {

                struct break_info {

                    break_info() : 
                        offset(0),
                        mark(0)
                    {
                    }
                    break_info(unsigned v) :
                        offset(v),
                        mark(0)
                    {
                    }

                    uint32_t offset;
                    uint32_t mark;
                   
                    bool operator<(break_info const &other) const
                    {
                        return offset < other.offset;
                    }
                };
               
                typedef std::vector<break_info> index_type;

                template<typename CharType>
                index_type map(boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc=std::locale());
                
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
                                || typeid(IteratorType) == typeid(char_type *)
                                || typeid(IteratorType) == typeid(char_type const *)
                            )
                            &&
                                b!=e
                          )
                        {
                            char_type const *begin = &*b;
                            char_type const *end = begin + (e-b);
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

            /// \endcond 


            //
            // Forward declarations to prevent collsion with boost::token_iterator
            //
            template<typename I> 
            class break_iterator; 
            template<typename I,typename V>
            class token_iterator; 

            ///
            /// \brief Class the holds boundary mapping of the text that can be used with iterators
            ///
            /// When the object is created in creates index and provides access to it with iterators.
            /// it is used mostly together with break_iterator and token_iterator. For each boundary point it
            /// provides the description mark of it that allows distinguish between different types of boundaries.
            /// For example it marks if sentence terminates because a mark like "?" or "." was found or because
            /// new line symbol is present in the text.
            ///
            /// These marks can be read out with token_iterator::mark() and break_iterator::mark() member functions.
            ///
            /// This class stores iterators to the original text, so you should be careful with iterators
            /// invalidation. If the iterators on original text are invalid you can't use this mapping any more.
            ///

            template<class RangeIterator>
            class mapping {
            public:
                ///
                /// Iterator type that is used to iterate over boundaries
                ///
                typedef RangeIterator iterator;
                ///
                /// Underlying iterator that is used to iterate original text.
                ///
                typedef typename RangeIterator::base_iterator base_iterator;
                ///
                /// The character type of the text
                ///
                typedef typename std::iterator_traits<base_iterator>::value_type char_type;

                ///
                /// Create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc
                ///
                mapping(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,0xFFFFFFFFu);
                }

                ///
                /// Create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc and set the boundaries
                /// mask to \a mask
                ///
                mapping(boundary_type type,base_iterator begin,base_iterator end,unsigned mask,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,mask);
                }

                ///
                /// Create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc
                ///
                void map(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,0xFFFFFFFFu);
                }

                ///
                /// Create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc, and set a mask to \a mask
                ///
                void map(boundary_type type,base_iterator begin,base_iterator end,unsigned mask,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,mask);
                }

                ///
                /// Default constructor of empty mapping
                ///

                mapping()
                {
                    mask_=0xFFFFFFFF;
                }

                ///
                /// Copy the mapping, note, you can copy the mapping that is used for token_iterator to break_iterator and vise versa.
                ///
                template<typename ORangeIterator>
                mapping(mapping<ORangeIterator> const &other) :
                    index_(other.index_),
                    begin_(other.begin_),
                    end_(other.end_),
                    mask_(other.mask_)
                {
                }

                ///
                /// Swap the mappings, note, you swap the mappings between those that are used for token_iterator to break_iterator and vise versa.
                /// This operation invalidates all iterators.
                ///
                template<typename ORangeIterator>
                void swap(mapping<ORangeIterator> &other)
                {
                    index_.swap(other.index_),
                    std::swap(begin_,other.begin_);
                    std::swap(end_,other.end_);
                    std::swap(mask_,other.mask_);
                }

                ///
                /// Copy the mapping, note, you can copy the mapping that is used for token_iterator to break_iterator and vise versa.
                ///
                template<typename ORangeIterator>
                mapping const &operator=(mapping<ORangeIterator> const &other)
                {
                    index_=other.index_;
                    begin_=other.begin_;
                    end_=other.end_;
                    mask_=other.mask_;
                }

                ///
                /// Get current boundary mask
                ///
                unsigned mask() const
                {
                    return mask_;
                }
                ///
                /// Set current boundary mask.
                ///
                /// This mask provides fine grained control on the type of boundaries and tokens you need to relate to. For example, if 
                /// you want to find sentence breaks that are caused only by terminator like "." or "?" and ignore new lines, you can set the mask
                /// value sentence_term and break iterator would iterate only over boundaries that much this mask.
                ///
                /// Note: the beginning of the text and the end of the text are always considered legal boundaries regardless if they have
                /// a mark that fits the mask.
                ///
                /// For token iterator it means which kind of tokens should be selected. Please note that token iterator generally selects the
                /// biggest amount of text that has specific mark. This is especially relevant for word boundary analysis.
                ///
                /// For example: if you set mask to word_any (selects numbers, letters) then when you iterate Over "To be, or not to be?" You would 
                /// get "To", "be", "or", "not", "to", "be". You can request from token iterator to use wider type of selection by
                /// calling token_iterator::full_select(true) so it would select only "To", " be", ", or", " not", " to", " be" tokens. All depends
                /// on your actual needs. For word selection you would probably want the first (default) and for sentence selection the second.
                ///
                /// Changing a mask does not invalidate current iterators but all new created iterators would not be compatible with old ones
                /// So you can't compare them, be careful with it.
                ///
                void mask(unsigned u)
                {
                    mask_ = u;
                }

                ///
                /// Get \a begin iterator used when object was created
                ///
                RangeIterator begin() const
                {
                    return RangeIterator(*this,true,mask_);
                }
                ///
                /// Get \a end iterator used when object was created
                ///
                RangeIterator end() const
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
            /// \brief token_iterator is an iterator that returns text chunks between boundary positions
            ///
            /// Token iterator may behave in two different ways: select specific tokens in only tide way and
            /// select them widely. For tide selection (default) it would not return text chunks that
            /// do not fit the selection mask. For example, for word iteration with mask "word_letters"
            /// for text "I met him at 7" it would return "I", "met", "him", "at" ignoring white spaces
            /// punctuation and numbers, But sometimes, you need to perform full selection of almost entry text
            /// for example for sentence boundaries and sentence_term mask you may want to specify full_select(true), 
            /// So "Hello! How<LF>are you?" would return you biggest possible chunks "Hello!", " How<LF>are you?".
            ///
            
            template<
                typename IteratorType,
                typename ValueType = std::basic_string<typename std::iterator_traits<IteratorType>::value_type> 
            >
            class token_iterator : public std::iterator<std::bidirectional_iterator_tag,ValueType> 
            {
            public:
                ///
                /// The character type of the text
                ///
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                ///
                /// Underlying iterator that is used to iterate original text.
                ///
                typedef IteratorType base_iterator;
                ///
                /// The type of mapping that iterator can iterate over it
                /// 
                typedef mapping<token_iterator<IteratorType,ValueType> > mapping_type;
                                
                ///
                /// Default constructor
                ///
                token_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0xFFFFFFFF),
                    full_select_(false)
                {
                }

                ///
                /// set position of the token iterator to the location of underlying iterator.
                ///
                /// This operator sets the token iterator to first token following that position. For example:
                ///
                /// For word boundary with "word_any" mask:
                ///
                /// - "to| be or ", would point to "be",
                /// - "t|o be or ", would point to "to",
                /// - "to be or| ", would point to end.
                ///
                /// \a p - should be in range of the original mapping.
                ///
                
                token_iterator const &operator=(IteratorType p)
                {
                    unsigned dist=std::distance(map_->begin_,p);
                    impl::index_type::const_iterator b=map_->index_.begin(),e=map_->index_.end();
                    impl::index_type::const_iterator 
                        bound=std::upper_bound(b,e,impl::break_info(dist));
                    while(bound != e && (bound->mark & mask_)==0)
                        bound++;
                    offset_ = bound - b;
                    return *this;
                }

                ///
                /// Create token iterator for mapping \a map with location at begin or end according to value of flag \a begin,
                /// and a mask \a mask
                ///
                /// It is strongly recommended to use map.begin(), map.end() instead.
                ///
                token_iterator(mapping_type const &map,bool begin,unsigned mask) :
                    map_(&map),
                    mask_(mask),
                    full_select_(false)
                {
                    if(begin) {
                        offset_ = 0;
                        next();
                    }
                    else
                        offset_=map_->index_.size();
                }
                ///
                /// Copy constructor
                ///
                token_iterator(token_iterator const &other) :
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_),
                    full_select_(other.full_select_)
                {
                }

                ///
                /// Assignment operator
                ///
                token_iterator const &operator=(token_iterator const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_=other.mask_;
                        full_select_ = other.full_select_;
                    }
                    return *this;
                }

                ///
                /// Return the token the iterator points it. Iterator must not point to the
                /// end of the range. Throws std::out_of_range exception
                ///
                /// Note, returned value is not lvalue, you can't use this iterator to assign new values to text.
                /// 
                ValueType operator*() const
                {
                    if(offset_ < 1 || offset_ >= map_->index_.size())
                        throw std::out_of_range("Invalid token iterator location");
                    unsigned pos=offset_-1;
                    if(full_select_)
                        while(!valid_offset(pos))
                            pos--;
                    base_iterator b=map_->begin_;
                    unsigned b_off = map_->index_[pos].offset;
                    std::advance(b,b_off);
                    base_iterator e=b;
                    unsigned e_off = map_->index_[offset_].offset;
                    std::advance(e,e_off-b_off);
                    return ValueType(b,e);
                }

                ///
                /// Increment operator
                ///
                token_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                ///
                /// Decrement operator
                ///
                token_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                ///
                /// Increment operator
                ///
                token_iterator operator++(int unused) 
                {
                    token_iterator tmp(*this);
                    next();
                    return tmp;
                }

                ///
                /// Decrement operator
                ///
                token_iterator operator--(int unused) 
                {
                    token_iterator tmp(*this);
                    prev();
                    return tmp;
                }

                ///
                /// Get full selection flag, see description of token_iterator
                ///
                bool full_select() const
                {
                    return full_select_;
                }
                ///
                /// Set full selection flag, see description of token_iterator
                ///
                void full_select(bool fs)
                {
                    full_select_ = fs;
                }
                
                ///
                /// Compare two iterators. They equal if they point to same map, have same position and same mask
                ///
                bool operator==(token_iterator const &other) const
                {
                    return  map_ == other.map_ 
                            && offset_==other.offset_ 
                            && mask_ == other.mask_;
                }

                ///
                /// Opposite of ===
                ///
                bool operator!=(token_iterator const &other) const
                {
                    return !(*this==other);
                }

                ///
                /// Return the mark that token iterator points at. See description of mapping class and various boundary flags
                ///
                unsigned mark() const
                {
                    return map_->index_.at(offset_).mark;
                }

            private:
                
                bool valid_offset(unsigned offset) const
                {
                    return  offset == 0 
                            || offset == map_->index_.size()
                            || (map_->index_[offset].mark & mask_)!=0;
                }

                bool at_end() const
                {
                    return !map_ || offset_>=map_->index_.size();
                }
                
                void next()
                {
                    while(offset_ < map_->index_.size()) {
                        offset_++;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                
                void prev()
                {
                    while(offset_ > 0) {
                        offset_ --;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                
                mapping_type const * map_;
                size_t offset_;
                unsigned mask_;
                uint32_t full_select_ : 1;
                uint32_t reserved_ : 31;
            };


            ///
            /// \brief break_iterator is bidirectional iterator that returns text boundary positions
            ///
            /// It returns rather iterators to break position then text chunks themselves. It stops only
            /// on boundaries that their marks fit the required mask. Also beginning of text and end of 
            /// text are valid boundaries regardless their marks.
            ///
            /// Please note for text in range [text_begin,text_end) and break_iterator it over it
            /// in range [begin,end): if *it==text_end then it!=end. And if it==end then *it is invalid.
            /// Thus for example for work iterator over text "hello", break iterator returns at beginning
            /// text_begin ("|hello"), then text_end ("hello|") and then it points to end.
            ///
            template<typename IteratorType>
            class break_iterator : public std::iterator<std::bidirectional_iterator_tag,IteratorType> 
            {
            public:
                ///
                /// The character type of the text
                ///
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                ///
                /// Underlying iterator that is used to iterate original text.
                ///
                typedef IteratorType base_iterator;
                ///
                /// The type of mapping that iterator can iterate over it
                /// 
                typedef mapping<break_iterator<IteratorType> > mapping_type;
                
                ///
                /// Default constructor
                ///
                break_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0xFFFFFFFF)
                {
                }

                ///
                /// Copy constructor
                ///
                break_iterator(break_iterator const &other):
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_)
                {
                }
                
                ///
                /// Assignment operator
                ///
                break_iterator const &operator=(break_iterator const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_=other.mask_;
                    }
                    return *this;
                }

                ///
                /// Create break iterator for mapping \a map with location at begin or end according to value of flag \a begin,
                /// and a mask \a mask
                ///
                /// It is strongly recommended to use map.begin(), map.end() instead.
                ///
                break_iterator(mapping_type const &map,bool begin,unsigned mask) :
                    map_(&map),
                    mask_(mask)
                {
                    if(begin)
                        offset_ = 0;
                    else
                        offset_=map_->index_.size();
                }

                ///
                /// Compare two iterators. They equal if they point to same map, have same position and same mask
                ///
                bool operator==(break_iterator const &other) const
                {
                    return  map_ == other.map_ 
                            && offset_==other.offset_
                            && mask_==other.mask_;
                }

                ///
                /// Opposite of ===
                ///
                bool operator!=(break_iterator const &other) const
                {
                    return !(*this==other);
                }

                ///
                /// Return the mark that token iterator points at. See description of mapping class and various boundary flags
                ///
                unsigned mark() const
                {
                    return map_->index_.at(offset_).mark;
                }
 
                ///
                /// set position of the break_iterator to the location of underlying iterator.
                ///
                /// This operator sets the break_iterator to position of the iterator p or to the first valid following position
                /// For example:
                ///
                /// For word boundary:
                ///
                /// - "|to be or ", would point to "|to be or " 
                /// - "t|o be or ", would point to "to| be or " 
                ///
                /// \a p - should be in range of the original mapping.
                ///
                break_iterator const &operator=(base_iterator p)
                {
                    at_least(p);
                    return *this;
                }
                
                ///
                /// Return the underlying iterator that break_iterator points it. Iterator must not point to the
                /// end of the range, otherwise throws std::out_of_range exception
                ///
                /// Note, returned value is not lvalue, you can't use this iterator to change underlying iterators.
                /// 
                base_iterator operator*() const
                {
                    if(offset_ >=map_->index_.size())
                        throw std::out_of_range("Invalid position of break iterator");
                    base_iterator p = map_->begin_;
                    std::advance(p, map_->index_[offset_].offset);
                    return p;
                }

                ///
                /// Increment operator
                ///
                break_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                ///
                /// Decrement operator
                ///
                break_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                ///
                /// Increment operator
                ///
                break_iterator operator++(int unused) 
                {
                    break_iterator tmp(*this);
                    next();
                    return tmp;
                }

                ///
                /// Decrement operator
                ///
                break_iterator operator--(int unused) 
                {
                    break_iterator tmp(*this);
                    prev();
                    return tmp;
                }

            private:
                bool valid_offset(unsigned offset) const
                {
                    return  offset == 0 
                            || offset + 1 >= map_->index_.size() // last and first are always valid regardless of mark
                            || (map_->index_[offset].mark & mask_)!=0;
                }

                bool at_end() const
                {
                    return !map_ || offset_>=map_->index_.size();
                }
                
                void next()
                {
                    while(offset_ < map_->index_.size()) {
                        offset_++;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                void prev()
                {
                    while(offset_ > 0) {
                        offset_ --;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                
                void at_least(IteratorType p)
                {
                    unsigned diff =  std::distance(map_->begin_,p);

                    impl::index_type::const_iterator b=map_->index_.begin();
                    impl::index_type::const_iterator e=map_->index_.end();
                    impl::index_type::const_iterator ptr = std::lower_bound(b,e,impl::break_info(diff));

                    if(ptr==map_->index_.end())
                        offset_=map_->index_.size()-1;
                    else
                        offset_=ptr - map_->index_.begin();
                    
                    while(!valid_offset(offset_))
                        offset_ ++;
                }

                mapping_type const * map_;
                size_t offset_;
                unsigned mask_;
                uint32_t reserved_;
            };

            ///
            /// @}
            ///

        } // boundary

    } // locale
} // boost

///
/// \example boundary.cpp
/// Example of using boundary iterator
/// \example wboundary.cpp
/// Example of using boundary iterator over wide strings
///

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
