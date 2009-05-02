#ifndef CPPCMS_HTTP_FILE_H
#define CPPCMS_HTTP_FILE_H

namespace cgicc { class FormFile; }

namespace cppcms { namespace http {

	class file {
		std::string name_;
		std::string mime_;
		std::string filename_;
		std::string data_;

		fstream file_;
		ostringstream file_data_;
		bool saved_in_file_;
	public:
		std::string name() const;
		std::string mime() const;
		std::string filename() const;
		istream &data();
		size_t size() const;
	};

} } //::cppcms::http


#endif
