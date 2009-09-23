#define CPPCMS_SOURCE
#include "utf8_iterator.h"
#include "config.h"
#include "utf_iterator.h"
#include "noncopyable.h"
#include "locale_icu_locale.h"

#ifdef HAVE_ICU
#include <unicode/locid.h>
#include <unicode/brkiter.h>
#include <unicode/utext.h>
#endif
#include <stdexcept>
#include <typeinfo>


namespace cppcms { namespace utf8 { namespace details {
#ifdef HAVE_ICU


	/// Implementation

	class break_iterator_impl : util::noncopyable {
	public:
		break_iterator_impl(char const *b,char const *e,icu::BreakIterator *it,UText *text = 0,size_t pos = (size_t)(-1)) :
			begin_(b),
			end_(e),
			br_(it),
			text_(text)
		{
			if(text_ == 0) {
				UErrorCode status = U_ZERO_ERROR;
				text_=utext_openUTF8(0,begin_,end_-begin_,&status);
				
				if(!U_SUCCESS(status)) {
					delete br_;
					throw std::runtime_error(u_errorName(status));
				}
			}
				
			UErrorCode status = U_ZERO_ERROR;
			br_->setText(text_,status);

			if(!U_SUCCESS(status)) {
				utext_close(text_);
				delete br_;
				throw std::runtime_error(u_errorName(status));
			}

			if(pos!=(size_t)(-1))
				br_->isBoundary(pos);
			else
				first();
		}
		~break_iterator_impl()
		{
			utext_close(text_);
			delete br_;
		}
		break_iterator_impl *clone() const
		{
			UErrorCode status = U_ZERO_ERROR;
			UText *text=utext_clone(0,text_,false,true,&status);
			if(!U_SUCCESS(status)) {
				throw std::runtime_error(u_errorName(status));
			}
			return new break_iterator_impl(begin_,end_,br_->clone(),text,br_->current());
		}

		bool equal(break_iterator_impl const &other) const
		{
			return *br_ == *other.br_;
		}
		bool less(break_iterator_impl const &other) const
		{
			return br_->current() < other.br_->current();
		}

		char const *to_che(int32_t n) const
		{
			if(n == icu::BreakIterator::DONE || begin_ + n > end_)
				return end_;
			return begin_ + n;
		}

		char const *to_chb(int32_t n) const
		{
			if(n == icu::BreakIterator::DONE || begin_ + n < begin_)
				return begin_;
			return begin_ + n;
		}

		char const *curr() const
		{
			return to_che(br_->current());
		}
		char const *first()
		{
			return to_che(br_->first());
		}

		char const *next()
		{
			return to_che(br_->next());
		}

		char const *last()
		{
			return to_chb(br_->last());
		}
		char const *prev()
		{
			return to_chb(br_->previous());
		}
		void curr(char const *pos)
		{
			br_->isBoundary(pos - begin_);
		}
		char const *following(char const *pos)
		{
			return to_che(br_->following(pos - begin_));
		}
		char const *preceding(char const *pos)
		{
			return to_chb(br_->preceding(pos - begin_));
		}

		char const *move(int n)
		{
			if(n>0)
				return to_che(br_->next(n));
			else
				return to_chb(br_->next(n));
		}

		char const *begin_;
		char const *end_;
		icu::BreakIterator *br_;
		UText *text_;
	};


	/// Actual Iterator	


	namespace {
		icu::Locale const &icu_locale(std::locale const &l)
		{
			try {
				return std::use_facet<locale::icu_locale>(l).get();
			}
			catch(std::bad_cast const &e) {
				return icu::Locale::getDefault();
			}
		}
		icu::BreakIterator *create_iterator(	std::locale const &l,
							icu::BreakIterator *(*factory)(icu::Locale const &l,UErrorCode &err))
		{
			icu::Locale const &loc=icu_locale(l);

			UErrorCode status = U_ZERO_ERROR;
			icu::BreakIterator *br = factory(loc,status);
			if(!U_SUCCESS(status)) {
				delete br;
				throw std::runtime_error(u_errorName(status));
			}
			return br;
		}
	}

	
	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,character_tag tag) :
		impl_(new break_iterator_impl(begin,end,create_iterator(loc, &icu::BreakIterator::createCharacterInstance)))
	{		
	}
	
	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,word_tag tag) :
		impl_(new break_iterator_impl(begin,end,create_iterator(loc, &icu::BreakIterator::createWordInstance)))
	{	
	}
	
	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,line_tag tag) :
		impl_(new break_iterator_impl(begin,end,create_iterator(loc, &icu::BreakIterator::createLineInstance)))
	{	
	}

	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,sentence_tag tag) :
		impl_(new break_iterator_impl(begin,end,create_iterator(loc, &icu::BreakIterator::createSentenceInstance)))
	{	
	}
	

#else  /// NO ICU
	
	class break_iterator_impl {
		code_point_iterator current_;
	public:
		typedef enum { charrecter, word, sentence, line } iter_type;

		iter_type type_;

		char const *begin_,*end_;
		std::ctype<wchar_t> const *wfacet_;
		std::ctype<char> const *facet_;


		break_iterator_impl *clone() const
		{
			return new break_iterator_impl(*this);
		}

		break_iterator_impl(char const *begin,char const *end,std::locale const &l,iter_type type) :
			current_(begin,end,l,code_point_tag()),
			type_(type),
			begin_(begin),
			end_(end),
			wfacet_(0),
			facet_(0)
		{
			if(std::has_facet<std::ctype<wchar_t> >(l)) {
				wfacet_ = & std::use_facet<std::ctype<wchar_t> >(l);
			}
			else {
				facet_ = & std::use_facet<std::ctype<char> >(l);;
			}
		}

		bool is(std::ctype_base::mask m)
		{
			char const *ptr=curr();
			if(ptr >= end_)
				return false;
			if(wfacet_) {
				wchar_t v=utf8::next(ptr,end_);
				return wfacet_->is(m,v);
			}
			else {
				return facet_->is(m,*ptr);
			}
		}
		bool is_alpha()
		{
			return is(std::ctype_base::alpha);
		}
		bool is_sentence_break()
		{
			return is(std::ctype_base::punct);
		}
		bool is_space()
		{
			return is(std::ctype_base::space);
		}

		bool equal(break_iterator_impl const &other) const
		{
			return current_ == other.current_;
		}
		bool less(break_iterator_impl const &other) const
		{
			return current_ < other.current_;
		}

		char const *curr() const
		{
			return current_.curr();
		}
		char const *next()
		{
			switch(type_) {
			case charrecter: return current_.next();
			case word: while(is_alpha()) current_.next(); return curr();
			case sentence: while(!is_sentence_break()) current_.next(); return current_.next(); 
			case line: while(!is_space()) current_.next(); return curr();
			}
		}
		char const *prev();
		{
			switch(type_) {
			case charrecter: return current_.prev();
			case word: while(is_alpha()) current_.prev(); return curr();
			case sentence: while(!is_sentence_break()) current_.prev(); return current_.prev(); 
			case line: while(!is_space()) current_.prev(); return curr();
			}
		}
		char const *first()
		{
			return current_.first();
		}
		char const *last()
		{
			return current_.last();
		}
		void curr(char const *iter)
		{
			switch(type_) {
			case charrecter: current_.curr(iter); break;
			default:
				current_.curr();
				next();
				prev();
			}
		}
		char const *following(char const *pos)
		{
			switch(type_) {
			case charrecter: return current_.following(pos);
			default:
				current_.curr();
				return next();
			}
		}

		char const *preceding(char const *pos)
		{
			switch(type_) {
			case charrecter: return current_.preceding(pos);
			default:
				current_.curr();
				return prev();
			}
		}
		char const *move(int steps)
		{
			while(steps > 0) {
				next();
				steps --;
			}
			while(steps < 0) {
				prev();
				steps ++;
			}
			return curr();
		}
	};
	
	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,character_tag tag) :
		impl_(new break_iterator_impl(begin,end,loc,break_iterator_impl::charrecter))
	{		
	}
	
	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,word_tag tag) :
		impl_(new break_iterator_impl(begin,end,loc,break_iterator_impl::word))
	{	
	}
	
	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,line_tag tag) :
		impl_(new break_iterator_impl(begin,end,loc,break_iterator_impl::line))
	{	
	}

	template<>
	break_iterator::break_iterator(char const *begin,char const *end,std::locale const &loc,sentence_tag tag) :
		impl_(new break_iterator_impl(begin,end,loc,break_iterator_impl::sentence))
	{	
	}


#endif

	break_iterator::break_iterator()
	{
	}
	break_iterator::~break_iterator()
	{
	}
	break_iterator::break_iterator(break_iterator const &other) : impl_(other.impl_)
	{
	}
	break_iterator const &break_iterator::operator=(break_iterator const &other)
	{
		impl_ = other.impl_;
		return *this;
	}
	bool break_iterator::equal(break_iterator const &other) const
	{
		if(impl_.get() && other.impl_.get())
			return impl_->equal(*other.impl_);
		if(!impl_.get() && !other.impl_.get())
			return true;
		return false;
	}
	bool break_iterator::less(break_iterator const &other) const
	{
		if(impl_.get() && other.impl_.get())
			return impl_->less(*other.impl_);
		return false;
	}

	char const *break_iterator::move(int n) { return impl_->move(n); }
	char const *break_iterator::next() { return impl_->next(); }
	char const *break_iterator::first() { return impl_->first(); }
	char const *break_iterator::prev() { return impl_->prev(); }
	char const *break_iterator::last() { return impl_->last(); }
	char const *break_iterator::curr() const { return impl_->curr(); }
	char const *break_iterator::following(char const *p) { return impl_->following(p); }
	char const *break_iterator::preceding(char const *p) { return impl_->preceding(p); }
	void break_iterator::curr(char const *p) { impl_->curr(p); }

}}} // cppcms::utf8::details
