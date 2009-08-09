#ifndef CPPCMS_LOCALE_INFO_H
#define CPPCMS_LOCALE_INFO_H

#include "defs.h"
#include "hold_ptr.h"
#include <locale>
#include <string>

namespace cppcms { namespace locale {

	class CPPCMS_API info : public std::locale::facet {
	public:
		static std::locale::id id;
		info(std::string name,std::size_t refs=0);
		~info();

		std::string name() const ;

		std::string language() const;
		std::string territory() const;
		std::string encoding() const;
		std::string variant() const;

		bool is_utf8() const;
	private:
		struct data;
		util::hold_ptr<data> d;
		std::string name_;
		std::string language_;
		std::string territory_;
		std::string encoding_;
		std::string variant_;
		bool is_utf8_;

	};


} } // cppcms::locale

#endif
