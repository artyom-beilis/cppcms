#ifndef CPPCMS_FORM_H
#define CPPCMS_FORM_H

namespace cppcms {

class base_form_entry : public boost::noncopyable {
	char const *id;
	char const *name
public:
	base_form_entry(base_form *f,char const *_id,char const *_name) : id(_id),name(w->gettext(_name)) { f->add(this); }
	virtual string render_html_input() = 0;
	virtual string get_title() = 0;
	virtual bool validate() = 0
	virtual ~base_form_entry(){};
};

class form_int : public base_form_entry {
	bool test;
	int low,high;
	string id,name;
public:
	int value;
	Integer(char const *id,char const *name,int default_val=0) : test(false), value(default_val) {};
	Integer(string id,string name,int l,int h) : test(true),low(l),high(h) {};
	virtual bool validate();
private:

};

class base_form : public boost::noncopyable {
	vector<base_form_entry *> this_methods
protected:
	void add(base_form_entry *entry);
	base_form *operator &(base_form_entry *entry) { add(entry) };
};

struct String {
	std::string value;
	String();
	String(int min_len);
};

}

#endif
