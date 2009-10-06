#define CPPCMS_SOURCE
#include "locale_collate.h"
#include "locale_icu_locale.h"
#include "noncopyable.h"
#include "icu_util.h"

#include <stdexcept>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <unicode/coll.h>


namespace cppcms {
namespace locale {

	class collate_impl : public util::noncopyable {
	public:
		typedef collate::level_type level_type;

		int compare(level_type level,std::string const &l,std::string const &r) const
		{

			UErrorCode status=U_ZERO_ERROR;
			
			icu::UnicodeString left(impl::std_to_icu(l,locale_));
			icu::UnicodeString right(impl::std_to_icu(r,locale_));

			int res = get(level)->compare(left,right,status);
				
			if(U_FAILURE(status))
				throw std::runtime_error(std::string("collation failed:") + u_errorName(status));
			if(res<0)
				return -1;
			else if(res > 0)
				return 1;
			return 0;
		}

		std::string transform(level_type level,std::string const &s) const
		{
			icu::UnicodeString us(impl::std_to_icu(s,locale_));
			std::string tmp;
			tmp.resize(s.size());
			boost::shared_ptr<icu::Collator> collator=get(level);
			int len = collator->getSortKey(us,reinterpret_cast<uint8_t *>(&tmp[0]),tmp.size());
			if(len > int(tmp.size())) {
				tmp.resize(len);
				collator->getSortKey(us,reinterpret_cast<uint8_t *>(&tmp[0]),tmp.size());
			}
			else 
				tmp.resize(len);
			return tmp;
		}

		collate_impl(std::locale const &l) : locale_(l)
		{
			icu_locale const &iculoc=std::use_facet<icu_locale>(locale_);
			static const icu::Collator::ECollationStrength levels[collators_size] = 
			{ 
				icu::Collator::PRIMARY,
				icu::Collator::SECONDARY,
				icu::Collator::TERTIARY,
				icu::Collator::QUATERNARY
			};
			
			for(int i=0;i<collators_size;i++) {

				UErrorCode status=U_ZERO_ERROR;

				collators_[i].reset(icu::Collator::createInstance(iculoc.get(),status));

				if(U_FAILURE(status))
					throw std::runtime_error(std::string("collation failed:") + u_errorName(status));

				collators_[i]->setStrength(levels[i]);
			}

		}

	private:
		
		static const int collators_size = 4;

		boost::shared_ptr<icu::Collator> get(level_type level) const
		{
			int l=int(level);
			if(l < 0)
				l=0;
			else if(l >= collators_size)
				l = collators_size-1;
			return collators_[l];
		}


		boost::shared_ptr<icu::Collator> collators_[collators_size];
		std::locale locale_;
	};

/////////

	collate::collate(collate_impl *impl,size_t refs) : std::collate<char>(refs),impl_(impl) 
	{
	}
	collate::~collate()
	{
	}
	int collate::compare(level_type level,std::string const &l,std::string const &r) const
	{
		return impl_->compare(level,l,r);
	}
	int collate::compare(	level_type level,
				char const *p1_start,char const *p1_end,
				char const *p2_start,char const *p2_end) const
	{
		return impl_->compare(level,std::string(p1_start,p1_end),std::string(p2_start,p2_end));
	}
	std::string collate::transform(level_type level,std::string const &s) const
	{
		return impl_->transform(level,s);
	}
	std::string collate::transform(level_type level,char const *b,char const *e) const
	{
		return impl_->transform(level,std::string(b,e));
	}

	long collate::hash(level_type level,std::string const &s) const
	{
		boost::hash<std::string> hasher;
		return hasher(transform(level,s));
	}
	long collate::hash(level_type level,char const *b,char const *e) const
	{
		boost::hash<std::string> hasher;
		return hasher(transform(level,b,e));
	}

	collate *collate::create(std::locale const &l)
	{
		return new collate(new collate_impl(l));
	}


} } // cppcms::locale
