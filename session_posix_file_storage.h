#ifndef CPPCMS_SESSION_POSIX_FILE_STORAGE_H
#define CPPCMS_SESSION_POSIX_FILE_STORAGE_H


namespace cppcms {
namespace sessions {
	class CPPCMS_API session_file_storage : public session_storage {
	public:
		session_file_storage(std::string path,int concurrency_hint);
		virtual ~session_file_storage();
		virtual void save(std::string const &sid,time_t timeout,std::string const &in);
		virtual bool load(std::string const &sid,time_t &timeout,std::string &out);
		virtual void remove(std::string const &sid);
	private:
		struct locked_file;
		bool read_timestamp(int fd);
		bool read_from_file(int fd,time_t &timeout,std::string &data);
		void save_to_file(int fd,time_t timeout,std::string const &in);
		bool read_all(int fd,void *vbuf,int n);
		bool write_all(int fd,void const *vbuf,int n);
		void gc();
	}
}
}

#endif
