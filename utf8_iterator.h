#ifndef CPPCMS_UTF8_ITERATOR_H
#define CPPCMS_UTF8_ITERATOR_H

#include "defs.h"
#include "clone_ptr.h"
#include <locale>
#include <string>

namespace cppcms {

	namespace utf8 {

		namespace details {

			struct code_point_tag {};
			struct character_tag {};
			struct word_tag {};
			struct sentence_tag {};
			struct line_tag {};


			template<typename T>
			struct string_traits;

			template<typename TraitsT,typename AllocT>
			struct string_traits<std::basic_string<char,TraitsT,AllocT> &>
			{
				typedef typename std::basic_string<char,TraitsT,AllocT> string_type;
				typedef typename string_type::iterator iterator_type;
				typedef string_type *pointer_type;
			};
			
			template<typename TraitsT,typename AllocT>
			struct string_traits<std::basic_string<char,TraitsT,AllocT> const &>
			{
				typedef typename std::basic_string<char,TraitsT,AllocT> string_type;
				typedef typename string_type::const_iterator iterator_type;
				typedef string_type const *pointer_type;
			};

			class codepoint_iterator {
			public:
				codepoint_iterator() : begin_(0), end_(0), current_(0)
				{
				}
				template<typename Tag>
				codepoint_iterator(char const *begin,char const *end,std::locale const &loc,Tag t);

				bool equal(codepoint_iterator const &other) const
				{
					return current_ == other.current_;
				}

				bool less(codepoint_iterator const &other) const
				{
					return current_ < other.current_;
				}

				char const *curr() const
				{
					return current_;
				}
				
				char const *next()
				{
					unsigned char tmp;
					while(current_ < end_ && ((tmp = *current_++) & 0xC0) == 0xC0 || tmp == 0x80)
						;
					return current_;

				}

				char const *prev()
				{
					while(current_ > begin_) {
						unsigned char c = * (--current_);
						if( (c & 0xC0) != 0x80 )// 10xxxxxx -- utf8 part
							break;
					}
					return current_;				 
				}

				char const *first()
				{
					current_ = begin_;
					return current_;
				}

				char const *last()
				{
					current_ = end_;
					return current_;
				}

				void curr(char const *iter)
				{
					if(iter <= begin_)
						current_ = begin_;
					else if(iter >= end_) {
						current_ = end_;
					}
					while( current_ != end_ && ((unsigned char)(*current_)) & 0xC0 == 0x80)
						current_++;
				}

				char const *following(char const *pos)
				{
					curr(pos);
					return next();
				}
				
				char const *preceding(char const *pos)
				{
					curr(pos);
					return prev();
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

			private:
				char const *begin_;
				char const *end_;
				char const *current_;
			};
			
			template<>
			inline codepoint_iterator::codepoint_iterator(	char const *begin,
									char const *end,
									std::locale const &loc,
									code_point_tag unused) :
					begin_(begin),
					end_(end),
					current_(begin)
			{
			}
			
			class break_iterator_impl;

			class CPPCMS_API break_iterator {
			public:
				break_iterator();
				template<typename Tag>
				break_iterator(char const *begin,char const *end,std::locale const &loc,Tag t);
				break_iterator(break_iterator const &);
				break_iterator const &operator = (break_iterator const &);
				~break_iterator();

				bool equal(break_iterator const &other) const;
				bool less(break_iterator const &other) const;

				char const *curr() const;
				char const *next();
				char const *prev();
				char const *first();
				char const *last();
				void curr(char const *iter);
				char const *following(char const *pos);
				char const *preceding(char const *pos);
				char const *move(int n);
			private:
				util::clone_ptr<break_iterator_impl> impl_;

			};
			
			template<>
			CPPCMS_API break_iterator::break_iterator(	char const *begin,
									char const *end,
									std::locale const &l,
									character_tag tag);
			template<>
			CPPCMS_API break_iterator::break_iterator(	char const *begin,
									char const *end,
									std::locale const &l,
									word_tag tag);
			template<>
			CPPCMS_API break_iterator::break_iterator(	char const *begin,
									char const *end,
									std::locale const &l,
									sentence_tag tag);
			template<>
			CPPCMS_API break_iterator::break_iterator(	char const *begin,
									char const *end,
									std::locale const &l,
									line_tag tag);

		} // details

		

		
		template<typename StringType,typename WalkerType,typename TagType>
		class base_iterator {
		public:
			typedef typename details::string_traits<StringType>::iterator_type iterator_type;
			typename details::string_traits<StringType>::pointer_type pointer_type;

			
			bool operator==(base_iterator const &other) const
			{
				return walker_.equal(other.walker_);
			}
			
			bool operator!=(base_iterator const &other) const
			{
				return !walker_.equal(other.walker_);
			}
			
			bool operator< (base_iterator const &other) const
			{
				return walker_.less(other.walker_);
			}
			
			bool operator> (base_iterator const &other) const
			{
				return other.walker_.less(walker_);
			}
			
			bool operator>= (base_iterator const &other) const
			{
				return !walker_.less(other.walker_);
			}
			
			bool operator<= (base_iterator const &other) const
			{
				return !other.walker_.less(walker_);
			}

			base_iterator(StringType str,std::locale const &l = std::locale()) :
				walker_(str.data(),str.data()+str.size(),l,TagType()),
				begin_(str.begin()),
				cbegin_(str.data())
			{
			}
			
			base_iterator(iterator_type begin,iterator_type end,std::locale const &l = std::locale()) :
				walker_(&*begin,&*end,l,TagType()),
				begin_(begin),
				cbegin_(&*begin)
			{
			}

			base_iterator(){}

			iterator_type operator*() const
			{
				return to_iterator(walker_.curr());
			}
			
			iterator_type next()
			{
				return to_iterator(walker_.next());

			}

			iterator_type prev()
			{
				return to_iterator(walker_.prev());
			}

			iterator_type first()
			{
				return to_iterator(walker_.first());
			}

			iterator_type last()
			{
				return to_iterator(walker_.last());
			}

			iterator_type to_iterator(char const *ptr) const
			{
				return begin_ + (ptr - cbegin_);
			}


			base_iterator const &operator=(iterator_type iter)
			{
				walker_.curr(iter - begin_);
				return *this;
			}

			iterator_type following(iterator_type pos)
			{
				return to_iterator(walker_.following(pos - begin_));
			}
			
			iterator_type preceding(iterator_type pos)
			{
				return to_iterator(walker_.preceding(pos - begin_));
			}

			base_iterator operator++(int unused)
			{
				base_iterator tmp(*this);
				next();
				return tmp;
			}

			base_iterator &operator++()
			{
				next();
				return *this;
			}
			
			base_iterator operator--(int unused)
			{
				base_iterator tmp(*this);
				prev();
				return tmp;
			}

			base_iterator &operator--()
			{
				prev();
				return *this;
			}

			base_iterator const &operator+=(int n)
			{
				walker_.move(n);
				return *this;
			}
			base_iterator const &operator-=(int n)
			{
				walker_.move(-n);
				return *this;
			}

		private:
			WalkerType walker_;
			iterator_type begin_;
			char const *cbegin_;
		};

		template<typename StringType,typename WalkerType,typename TagType>
		base_iterator<StringType,WalkerType,TagType> operator+(
				base_iterator<StringType,WalkerType,TagType> const &iter,
				int n)
		{
			base_iterator<StringType,WalkerType,TagType> tmp=iter;
			tmp+=n;
			return tmp;
		}
		
		template<typename StringType,typename WalkerType,typename TagType>
		base_iterator<StringType,WalkerType,TagType> operator+(
				int n,
				base_iterator<StringType,WalkerType,TagType> const &iter)
		{
			base_iterator<StringType,WalkerType,TagType> tmp=iter;
			tmp+=n;
			return tmp;
		}
		
		template<typename StringType,typename WalkerType,typename TagType>
		base_iterator<StringType,WalkerType,TagType> operator-(
				base_iterator<StringType,WalkerType,TagType> const &iter,
				int n)
		{
			base_iterator<StringType,WalkerType,TagType> tmp=iter;
			tmp-=n;
			return tmp;
		}
		
		template<typename StringType,typename WalkerType,typename TagType>
		base_iterator<StringType,WalkerType,TagType> operator-(
				base_iterator<StringType,WalkerType,TagType> const &a,
				base_iterator<StringType,WalkerType,TagType> const &b)
		{
			int n=0;
			if(a  >  b)  {
				base_iterator<StringType,WalkerType,TagType> tmp=b;
				int n = 0;
				while(b < a) {
					++b;
					++n;
				}
			}
			else if( b > a ) {
				base_iterator<StringType,WalkerType,TagType> tmp=a;
				int n = 0;
				while(b > a) {
					++a;
					--n;
				}
			}
			return n;
		}

		typedef base_iterator<std::string &,details::codepoint_iterator,details::code_point_tag> code_point_iterator;
		typedef base_iterator<std::string const &,details::codepoint_iterator,details::code_point_tag> const_code_point_iterator;
		typedef base_iterator<std::string &,details::break_iterator,details::character_tag> character_iterator;
		typedef base_iterator<std::string const &,details::break_iterator,details::character_tag> const_character_iterator;
		
		typedef base_iterator<std::string &,details::break_iterator,details::word_tag> word_iterator;
		typedef base_iterator<std::string const &,details::break_iterator,details::word_tag> const_word_iterator;

		typedef base_iterator<std::string &,details::break_iterator,details::sentence_tag> sentence_iterator;
		typedef base_iterator<std::string const &,details::break_iterator,details::sentence_tag> const_sentence_iterator;

		typedef base_iterator<std::string &,details::break_iterator,details::line_tag> line_iterator;
		typedef base_iterator<std::string const &,details::break_iterator,details::line_tag> const_line_iterator;

	} // utf8

} // cppcms




#endif
