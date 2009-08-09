#ifndef CPPCMS_BASE_VIEW_H
#define CPPCMS_BASE_VIEW_H

#include "defs.h"

#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include <ctime>

#include "format.h"
#include "hold_ptr.h"

namespace cppcms {
	namespace http {
		class context;
	}
	namespace filters {


		
	} // filters


class CPPCMS_API base_view : util::noncopyable {
public:
	virtual void render();
	virtual ~base_view();


public: //Filters


	class CPPCMS_API intf {
		std::string format_;
	public:
		intf(std::string f);
		~intf();
		void operator()(std::ostream &out,int x) const;
	};
		
	class CPPCMS_API doublef {
		std::string format_;
	public:
		doublef(std::string f);
		~doublef();
		void operator()(std::ostream &out,double x) const;
	};

	class CPPCMS_API strftime {
		std::string format_;
	public:
		strftime(std::string f);
		~strftime();
		void operator()(std::ostream &out,std::tm const &t) const;
	};
		
	static void date(std::ostream &,std::tm const &t);
	static void time(std::ostream &,std::tm const &t);
	static void urlencode(std::ostream &, std::string const &s);
// TODO	static void js_urlencode(std::ostream &, std::string const &s);
	static void base64_encode(std::ostream &, std::string const &s);
	static void raw(std::ostream &out,std::string const &s);

	template<typename T>
	void escape(std::ostream &s,T const &v)
	{
		s<<v;
	};

	static void to_upper(std::ostream &,std::string const &s);
	static void to_lower(std::ostream &,std::string const &s);


protected:

	base_view(http::context &context,std::ostream &out);

	void set_domain(std::string domain);
	char const *gettext(char const *);
	char const *ngettext(char const *,char const *,int n);

	std::ostream &out();




private:
	struct data;
	util::hold_ptr<data> d;

};

template<>
void CPPCMS_API base_view::escape(std::ostream &,std::string const &s);

void CPPCMS_API operator<<(std::ostream &,std::tm const &t);



} // cppcms


#if defined(HAVE_CPP_0X_AUTO)
#	define CPPCMS_TYPEOF(x) auto
#elif defined(HAVE_CPP_0X_DECLTYPE)
#	define CPPCMS_TYPEOF(x) decltype(x)
#elif defined(HAVE_GCC_TYPEOF)
#	define CPPCMS_TYPEOF(x) typeof(x)
#elif defined(HAVE_UNDERSCORE_TYPEOF)
#	define CPPCMS_TYPEOF(x) __typeof__(x)
#else
#	define CPPCMS_TYPEOF(x) automatic_type_identification_is_not_supported_by_this_compiler
#endif


#endif
