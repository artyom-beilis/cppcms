#ifndef CPPCMS_LOCALE_ICU_H
#define CPPCMS_LOCALE_ICU_H

#include "defs.h"
#include <locale>

namespace cppcms {
namespace locale {

	#ifdef HAVE_ICU
	typedef icu::UnicodeString unicode_string;
	typedef icu::Locale unicode_locale;
	#else
	typedef std::basic_string<wchar_t> unicode_string;
	typedef std::locale unicode_locale;
	#endif


	class CPPCMS_API unicode : public std::locale::facet {
	public:
		static std::locale::id id;

		unicode(unicode_locale const &source,size_t refs=0);
		~unicode();

		unicode_locale const &get() const;

		template<typename Char>
		std::basic_string<Char> to_upper(std::basic_string<Char> const &str) const
		{
			to_std<Char>(to_uni(str).do_to_upper(get()));
		}

		template<typename Char>
		std::basic_string<Char> to_lower(std::basic_string<Char> const &str) const
		{
			to_std<Char>(to_uni(str).do_to_lower(get()));
		}

		template<typename Char>
		std::basic_string<Char> to_title(std::basic_string<Char> const &str) const
		{
			to_std<Char>(to_uni(str).do_to_title(get()));
		}

		enum {	norm_default,
			nfc,
			nfd,
			nfkc,
			nfkd
		} norm_type;

		template<typename Char>
		std::basic_string<Char> to_normal(std::basic_string<Char> const &str,norm_type how=normalize_default) const
		{
			to_std<Char>(do_to_normal(to_icu(str),how));
		}

		template<Char>
		std::basic_string<Char> to_std(unicode_string const &str);

		template<Char>
		unicode_string to_uni(std::basic_string<Char> const &str);

	private:

		
		unicode_string do_to_upper(unicode_string const &s) const;
		unicode_string do_to_lower(unicode_string const &s) const;
		unicode_string do_to_title(unicode_string const &s) const;
		unicode_string do_to_normal(unicode_string const &s) const;

		struct data;
		util::hold_ptr<data> d;

		unicode_locale locale_;
		bool utf8_;
			
	};

	template<>
	std::basic_string<char> CPPCMS_API unicode::to_std(unicode_string const &str);

	template<>
	unicode_string CPPCMS_API unicode::to_uni(std::basic_string<char> const &str);


	typedef enum {
		character,
		word,
		line,
		sentence
	} boundary_type;

	void CPPCMS_API std_locale_boundary(	char const *begin,
						char const *end,
						boundary_type how,
						std::vector<size_t> map_,
						std::locale const &loc);

	void CPPCMS_API utf8_boundary(	char const *begin,
					char const *end,
					boundary_type how,
					std::vector<size_t> map_,
					std::locale const &loc);

	void CPPCMS_API utf16_boundary(	uint16_t const *begin,
					uint16_t const *end,
					boundary_type how,
					std::vector<size_t> map_,
					std::locale const &loc);

	void CPPCMS_API utf32_boundary(	uint32_t const *begin,
					uint32_t const *end,
					boundary_type how,
					std::vector<size_t> map_,
					std::locale const &loc);



	
	template<typename StringType,typename Iter>
	class basic_boundary_iterator {
	public:
		Iter current()
		{
			if(pos_ < map_.size())
				return str_.begin() + map_[pos_];
			return str_.end();
		}
		Iter first()
		{
			pos_=0;
			return current();
		}
		Iter next()
		{
			if(pos_ < map_.size())
				pos_++;
			return current();
		}
		Iter perv()
		{
			if(pos_ > 0)
				pos_--;
			return current();
		}
		Iter previous(Iter pos)
		{
			pos_= std::lower_bound(map_.begin(),map_.end(),pos - str_.begin()) - map_.begin();
			return current();
		}
		Iter following(Iter pos)
		{
			pos_= std::upper_bound(map_.begin(),map_.end(),pos - str_.begin()) - map_.begin();
			return current();
		}
		basic_boundary_iterator<StringType,Iter> &operator++()
		{
			next();
			return *this;
		}
		basic_boundary_iterator<StringType,Iter> operator++(int unused)
		{
			basic_boundary_iterator<StringType,Iter> tmp(*this);
			next();
			return tmp;
		}
		basic_boundary_iterator<StringType,Iter> &operator--()
		{
			prev();
			return *this;
		}
		basic_boundary_iterator<StringType,Iter> operator--(int unused)
		{
			basic_boundary_iterator<StringType,Iter> tmp(*this);
			perv();
			return tmp;
		}


		basic_boundary_iterator(StringType &str,boundary_type how,std::locale const &loc) :
			pos_(0),
			str_(str)
		{
			switch(StringType) {
			case std::string:
				{
					char const *begin=str_.data();
					char const *end=begin+str_.size();
					if(!std::use_facet<info>(loc).is_utf8()) 
						std_locale_boundary(begin,end_,how,map_,loc);
					else  
						utf8_boundary(begin,end,how,map_,loc);
				}
				break;
			case std::basic_string<wchar_t>:
				{
					#if SIZEOF_WCHAR_T == 2
					uint16_t const *begin=reinterpret_cast<uint16_t const *>(str_.data());
					uint16_t const *end=begin+str_.size();
					utf16_boundary(begin,end,how,map_,loc);
					#else
					uint32_t const *begin=reinterpret_cast<uint32_t const *>(str_.data());
					uint32_t const *end=begin+str_.size();
					utf32_boundary(begin,end,how,map_,loc);
					#endif
				}
				break;
			case std::basic_string<uint16_t>:
		#ifdef HAVE_CPP0X_UXSTRING
			case std::basic_string<char16_t>:
		#endif
				{
					uint16_t const *begin=reinterpret_cast<uint16_t const *>(str_.data());
					uint16_t const *end=begin+str_.size();
					utf16_boundary(begin,end,how,map_,loc);
				}
				break;
			case std::basic_string<uint32_t>:
		#ifdef HAVE_CPP0X_UXSTRING
			case std::basic_string<char32_t>:
		#endif
				{
					uint32_t const *begin=reinterpret_cast<uint32_t const *>(str_.data());
					uint32_t const *end=begin+str_.size();
					utf32_boundary(begin,end,how,map_,loc);
					#endif
				}
				break;
			};
		}
	private:
		size_t pos_;
		std::vector<size_t> map_;
		StringType &str_;
	};


} // locale
} // cppcms


#endif
