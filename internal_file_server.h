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

	private:
		void serve_file(std::string file_name);
		void show404();
		void load_mime_types(std::string);

		std::string document_root_;
		typedef std::map<std::string,std::string> mime_type;
		mime_type mime_;
	};


}
}
