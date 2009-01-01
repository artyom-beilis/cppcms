#ifndef ARCHIVE_H
#define ARCHIVE_H
#include "cppcms_error.h"
#include <string>
#include <cstring>

namespace cppcms {

using namespace std;

namespace sessions {
	string sign(string const &val,string const &pkey);
	string check_sign(string const &val,string const &pkey);
}

class archive {
	string data;
	size_t ptr;
public:
	archive() { ptr=0; };
	archive(string const &s) : data(s) { ptr=0; };
	void set(string const &s) { data=s; ptr=0; };
	string &set() { ptr=0; return data; };
	void set(char const *ptr,size_t len) { data.assign(ptr,len); };
	string const &get() const { return data; };
	string &get() { return data; }
	template<typename T>
	archive &operator<<(T const &val) {
		size_t size=sizeof(T);
		data.append((char const *)&size,sizeof(size_t));
		data.append((char const *)&val,size);
		return *this;
	}
	archive &operator<<(string const &val) {
		size_t size=val.size();
		data.append((char const *)&size,sizeof(size_t));
		data.append(val.c_str(),size);
		return *this;
	}
	template<typename T>
	archive &operator>>(T &val)
	{
		if(ptr+sizeof(size_t)+sizeof(T)>data.size()) {
			throw cppcms_error("Format violation");
		}
		char const *start=data.c_str()+ptr;
		if(*(size_t const *)start!=sizeof(T)) {
			throw cppcms_error("Invalid size read");
		}
		start+=sizeof(size_t);

		memcpy(&val,start,sizeof(T));

		ptr+=sizeof(size_t)+sizeof(T);
		return *this;
	}
	archive &operator>>(string &val)
	{
		if(ptr+sizeof(size_t)>data.size()) {
			throw cppcms_error("Format violation");
		}
		char const *start=data.c_str()+ptr;
		size_t s=*(size_t const *)start;
		if(ptr+sizeof(size_t)+s>data.size()) {
			throw cppcms_error("String too long");
		}
		start+=sizeof(size_t);
		val=string(start,s);
		ptr+=sizeof(size_t)+s;
		return *this;
	}
};

class serializable {
public:
	virtual void load(archive &a) = 0;
	virtual void save(archive &a) const = 0;
	
	operator std::string() const 
	{
		return str();
	}
	serializable const &operator=(std::string const &s)
	{
		str(s);
		return *this;
	}

	void str(std::string const &s)
	{
		archive a(s);
		load(a);
	}	
	
	std::string str() const
	{
		archive a;
		save(a);
		string str;
		str.swap(a.get());
		return str;
	}

	virtual ~serializable() {};
};

}

#endif
