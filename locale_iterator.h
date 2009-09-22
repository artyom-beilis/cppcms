#ifndef CPPCMS_LOCALE_ITERATOR_H
#define CPPCMS_LOCALE_ITERATOR_H

#include "defs.h"
#include "clone_ptr.h"
#include "config.h"


namespace cppcms {
	namespace locale {

		typedef enum {
			break_codepoint,
			break_character,
			break_word,
			break_line,
			break_sentence
		} break_type;

		namespace detail {

			/// The implementation of break iterator

			class CPPCMS_API breaker {
			public:
				virtual size_t first() = 0;
				virtual size_t next() = 0;
				virtual size_t last() = 0;
				virtual size_t prev() = 0;
				virtual size_t curr() = 0;
				virtual size_t following(size_t pos) = 0;
				virtual size_t preceding(size_t pos) = 0;
				virtual breaker *clone() const = 0;

				virtual ~breaker();

				template<typename Char>
				static breaker *create(Char const *text,size_t length,std::locale const &,break_type how);
				

			protected:
				breaker();
			private:
				breaker(breaker const &other);

				breaker const &operator=(breaker const &);
			};

			template<>
			breaker CPPCMS_API *breaker::create(char const *text,size_t len,std::locale const &l,break_type how);

			template<>
			breaker CPPCMS_API *breaker::create(uint16_t const *text,size_t len,std::locale const &l,break_type how);

			template<>
			breaker CPPCMS_API *breaker::create(uint32_t const *text,size_t len,std::locale const &l,break_type how);
			
			#ifdef HAVE_STD_WSTRING
			template<>
			inline breaker *breaker::create(wchar_t const *text,size_t len,std::locale const &l,break_type how)
			{
				#if SIZEOF_WCHAR_T == 2
				return breaker(reinterpret_cast<uint16_t const *>(text),len,l,how);
				#else
				return breaker(reinterpret_cast<uint32_t const *>(text),len,l,how);
				#endif
			}
			#endif

			#ifdef HAVE_CPP0X_UXSTRING
			template<>
			inline breaker *breaker::create(char16_t const *text,size_t len,std::locale const &l,break_type how)
			{
				return breaker(reinterpret_cast<uint16_t const *>(text),len,l,how)
			}
			
			template<>
			inline breaker *breaker::create(char32_t const *text,size_t len,std::locale const &l,break_type how)
			{
				return breaker(reinterpret_cast<uint32_t const *>(text),len,l,how)
			}
			#endif
			

		}

		/// It is also can be used as input iterator
		template<typename Char>
		class break_iterator : std::iterator<std::input_iterator_tag,Char> {
		public:
			break_iterator(std::basic_string<Char> const &data,break_type how,std::locale const &l)
			{
				setup(data.data(),data.size(),l,how);
			}
			break_iterator(Char const *data,break_type how,std::locale const &loc)
			{
				Char const *tmp=data;
				while(*tmp) tmp++;
				setup(data,tmp-data,l,how);
			}
			break_iterator(Char const *data,size_t n,break_type how,std::locale const &loc)
			{
				setup(data,n,l,how);
			}

			/// Copy and Assignment are default one carried by clone_ptr
			
			bool operator==(break_iterator<Char> const &other) const
			{
				/// Default iterator is equal to end iterator
				if(!other_->get() && impl_.get() && impl_->curr() == size_)
					return true;
				if(!impl_.get() && other_->get() && other_.impl_->curr == other_.impl_->size_)
					return true;
				return bool(impl_->get()) == bool(impl_->get()) && impl_->curr() == other.impl_->curr();
			}
			
			bool operator!=(break_iterator<Char> const &other) const
			{
				return !(*this == other); 
			}
			
			bool operator==(break_iterator<Char> const &other)
			{
				return impl_->curr() == other.impl_->curr(); 
			}

			Char const &operator*() const
			{
				return data_ + impl_->curr();
			}

			break_iterator<Char> operator++(int unused)
			{
				break_iterator<Char> tmp(*this);
				impl_->next();
				return tmp;
			}

			break_iterator<Char> &operator++()
			{
				impl_->next();
				return *this;
			}

			break_iterator()
			{
				data_=0;
				size_=0;
			}
			
			size_t first() { return impl_->first(); }
			size_t next() { return impl_->next(); }
			size_t last() { return impl_->last(); }
			size_t prev() { return impl_->prev(); }
			size_t curr() { return impl_->curr(); }
			size_t following(size_t pos) { return impl_->following(pos); }
			size_t preceding(size_t pos) { return impl_->preceding(pos); }

		private:
			void setup(Char const *data,size_t n,break_type how,std::locale const &loc)
			{
				impl_.reset(details::breaker::create(data,n,l,how));
				data_=data;
				size_=n;
				prev_=next_=impl_->first();
			}

			Char const *data_;
			size_t size_;

			util::clone_ptr<detail::breaker> impl_;
		};

	}
}


#endif
