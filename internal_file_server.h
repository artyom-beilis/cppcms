#include "application.h"
#include "applications_pool.h"
#include <string>
#include <map>

namespace cppcms {
	class service;
namespace impl {
	class file_server : public application
	{
	public:
		file_server(cppcms::service &srv);
		~file_server();
		virtual void main(std::string file_name);

	private:
		void show404();
		void load_mime_types(std::string);
		bool canonical(std::string normal,std::string &real);
		bool check_in_document_root(std::string normal,std::string &real);
		int file_mode(std::string const &path);

		std::string document_root_;
		typedef std::map<std::string,std::string> mime_type;
		mime_type mime_;
	};


}
}
