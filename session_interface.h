#ifndef CPPCMS_SESSION_INTERFACE_H
#define CPPCMS_SESSION_INTERFACE_H

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

namespace cppcms {

class session_api;
class worker_thread;
class serializable;

class session_interface : private boost::noncopyable {
	struct entry {
		std::string value;
		bool exposed;
		entry(std::string v="",bool exp=false) : value(v) , exposed(exp) {}
		bool operator==(entry const &other) const 
		{
			return value==other.value && exposed==other.exposed;
		}
		bool operator!=(entry const &other) const 
		{
			return !(*this==other);
		}
	};
	typedef std::map<std::string,entry> data_t;
	data_t data,data_copy;
	worker_thread &worker;

	// Cached defaults
	int timeout_val_def;
	int how_def;

	// User Values
	int timeout_val;
	int how;

	// Information from session data
	time_t timeout_in;
	bool new_session;
	bool saved;

	int cookie_age();
	time_t   session_age();

	void check();
	bool load();
	void update_exposed(bool force=false); 

	std::string temp_cookie;

	boost::shared_ptr<session_api> storage;
	void set_session_cookie(int64_t age,std::string const &data,std::string const &key=std::string());

	void save_data(data_t const &data,std::string &s);
	void load_data(data_t &data,std::string const &s);

public:
	session_interface(worker_thread &w);
	bool is_set(std::string const &key);
	void del(std::string const &key);
	std::string &operator[](std::string const &);
	template<typename T>
	T get(std::string const &key) {
		return boost::lexical_cast<T>((*this)[key]);
	}
	template<typename T>
	void set(std::string const &key,T const &val) {
		(*this)[key]=boost::lexical_cast<std::string>(val);
	}
	
	void get(std::string const &key,serializable &);
	void set(std::string const &key,serializable const &);
	bool is_exposed(std::string const &key);
	void expose(std::string const &key,bool val=true);
	void hide(std::string const &key);


	void clear();

	enum { fixed, renew, browser };
	void set_age(int t);
	void set_expiration(int h);
	void set_age();
	void set_expiration();
	void save();

// Special interface
	void set_session_cookie(std::string const &data);
	void clear_session_cookie();
	std::string get_session_cookie();
	void set_api(boost::shared_ptr<session_api>);

void on_start();
	void on_end();
	worker_thread &get_worker();
};


} // cppcms


#endif
