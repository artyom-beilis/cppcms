#ifndef CPPCMS_HTTP_FILE_H
#define CPPCMS_HTTP_FILE_H

#include "defs.h"
#include "hold_ptr.h"
#include "noncopyable.h"
#include <sstream>
#include <fstream>

namespace cppcms { namespace http {

	class request;

	class CPPCMS_API file {
	public:
		std::string name() const;
		std::string mime() const;
		std::string filename() const;
		std::istream &data();
		size_t size() const;
		file();
		~file();
	private:
		std::string name_;
		std::string mime_;
		std::string filename_;
		size_t size_;

		std::fstream file_;
		std::stringstream file_data_;
		
		uint32_t saved_in_file_ : 1;
		uint32_t reserverd_ : 31;

		struct impl_data; // for future use
		util::hold_ptr<impl_data> d;
		friend class request;
	};


} } //::cppcms::http


#endif
