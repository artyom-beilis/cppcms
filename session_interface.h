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
	std::map<std::string,std::string> data,data_copy;
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

	int cookie_age();
	time_t   session_age();

	void check();
	bool load();
	void save();

	std::string temp_cookie;

	boost::shared_ptr<session_api> storage;
	void set_session_cookie(int64_t age,std::string const &data);

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


	void clear();

	enum { fixed, renew, browser };
	void set_age(int t) { timeout_val=t;}
	void set_expiration(int h) { how=h; };

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
