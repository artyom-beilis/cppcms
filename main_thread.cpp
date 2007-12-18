#include "main_thread.h"

#include "global_config.h"

#include "templates/look.h"
#include "templates.h"
#include "data.h"
#include "boost/format.hpp"
#include "text_tool.h"



void Main_Thread::init()
{
	url.init(this);

	url.add("^/?$",		BIND(&Main_Thread::show_main_page,this,"end"));
	url.add("^/from/(\\d+)$",BIND(&Main_Thread::show_main_page,this,$1));
	url.add("^/login$",	BIND(&Main_Thread::show_login,this));
	url.add("^/logout$",	BIND(&Main_Thread::show_logout,this));
	url.add("^/newpost$",	BIND(&Main_Thread::show_post_form,this));
	url.add("^/post$",	BIND(&Main_Thread::get_post_message,this));
	url.add("^/dologin$",	BIND(&Main_Thread::do_login,this));
	url.add("^/edit/(\\d+)$",BIND(&Main_Thread::edit_message,this,$1));

}

void Main_Thread::edit_message(string msg)
{
	// TODO
}


void Main_Thread::load_cookies()
{
	const vector<HTTPCookie> &cookies = env->getCookieList();
	unsigned int i;

	username.clear();
	visitor.clear();
	email.clear();
	vurl.clear();
	password.clear();

	for(i=0;i<cookies.size();i++) {
		if(cookies[i].getName()=="username") {
			username=cookies[i].getValue();
		}
		else if(cookies[i].getName()=="password") {
			password=cookies[i].getValue();
		}
		else if(cookies[i].getName()=="visitor") {
			visitor=cookies[i].getValue();
		}
		else if(cookies[i].getName()=="email") {
			email=cookies[i].getValue();
		}
		else if(cookies[i].getName()=="url") {
			vurl=cookies[i].getValue();
		}
	}
}

void Main_Thread::check_athentication_by_name(string name,string password)
{
	Users::username_c cur(users->username);

	if(cur==name){
		user_t const &user=cur;
		if(user.password==password.c_str()) {
			authenticated=true;
			user_id=user.id;
		}
		username=name;
	}
	else {
		username.clear();
		authenticated=false;
	}
	password.clear();
}

void Main_Thread::check_athentication()
{
	load_cookies();
	check_athentication_by_name(username,password);
}

void Main_Thread::load_inputs()
{

	const vector<FormEntry> &elements=cgi->getElements();
	unsigned i;

	page=0;
	message.clear();

	for(i=0;i<elements.size();i++) {
		string const &name=elements[i].getName();
		if(name=="message"){
			message=elements[i].getValue();
		}
		else if(name=="username"){
			new_username=elements[i].getValue();
		}
		else if(name=="password"){
			new_password=elements[i].getValue();
		}
	}
}

void Main_Thread::show_login()
{
	Content c(T_VAR_NUM);

	c[TV_title]="Login";
	c[TV_show_content]=TT_dologin;

	Renderer r(templates,TT_master,c);

	while(r.render(out)!=0);

}

void Main_Thread::do_login()
{
	load_inputs();

	check_athentication_by_name(new_username,new_password);

	if(authenticated) {
		set_header(new HTTPRedirectHeader("/site/"));
		HTTPCookie cookie_u("username",username,"","",7*24*3600,"/",false);
		response_header->setCookie(cookie_u);
		HTTPCookie cookie_p("password",new_password,"","",7*24*3600,"/",false);
		response_header->setCookie(cookie_p);
	}
	else {
		username="";
		password="";
		set_header(new HTTPRedirectHeader("/site/login"));
	}
};

void Main_Thread::show_logout()
{
	set_header(new HTTPRedirectHeader("/site/"));
	HTTPCookie cookie("username","","","",0,"/",false);
	response_header->setCookie(cookie);
	cookie.setName("password");
	cookie.setValue("");
	response_header->setCookie(cookie);
}

void Main_Thread::show_main_page(string from)
{
	check_athentication();

	Content c(T_VAR_NUM);
	Renderer t(templates,TT_master,c);

	c[TV_title]="Main page";
	c[TV_show_content]=TT_main;

	if(authenticated) {
		c[TV_username]=username;
	}

	Messages::id_c cur(all_messages->id);

	if(from=="end") {
		cur.end();
	}
	else {
		int from_id=atoi(from.c_str());
		cur<from_id;
	}


	int id;
	string content;
	int counter=0;
	while((id=t.render(out))!=0) {
		if(id==TV_get_message){
			if(cur && counter<10){
				message_t const &message=cur;
				c[TV_new_message]=1;
				char buf[20];
				snprintf(buf,20,"%d",message.id);
				c[TV_message_id]=buf;

				string intext;

				texts->get(message.text_id,intext);
				Text_Tool conv;
				conv.markdown2html(intext,content);

				c[TV_message_body]=content.c_str();

				Users::id_c ucur(users->id);
				ucur==message.user_id;
				user_t const &user=ucur;
				c[TV_author]=user.username.c_str();
				cur.next();
				counter++;
			}
			else {
				c[TV_new_message]=0;
			}
		}
	}
}

void Main_Thread::show_post_form()
{
	check_athentication();
	if(authenticated) {
		Content c(T_VAR_NUM);

		c[TV_title]="New message";
		c[TV_show_content]=TT_post;
		Renderer t(templates,TT_master,c);
		while(t.render(out));
	}
	else {
		set_header(new HTTPRedirectHeader("/site/login"));

	}
}

void Main_Thread::get_post_message()
{
	check_athentication();

	load_inputs();

	if(!authenticated){
		set_header(new HTTPRedirectHeader("/site/login"));
		return;
	}


	message_t msg;
	msg.text_id=texts->add(message);
	msg.user_id=user_id;
	all_messages->id.add(msg);

	set_header(new HTTPRedirectHeader("/site/"));
}

void Main_Thread::show_page()
{
	url.parse();
}
void Main_Thread::main()
{
	try {
		show_page();
	}
	catch (DbException &err) {
		throw HTTP_Error(err.what());
	}
	catch(char const *s) {
		throw HTTP_Error(s);
	}
}
