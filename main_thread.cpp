#include "main_thread.h"

#include "global_config.h"

#include "templates/look.h"
#include "templates.h"

extern Templates_Set templates;

void Main_Thread::init()
{
	url.init(this);
	url.reserve(10);

	url.add("^/?$",		BIND(&Main_Thread::show_main_page,this,"end"));
	url.add("^/from/(\\d+)$",BIND(&Main_Thread::show_main_page,this,$1));
	url.add("^/login$",	BIND(&Main_Thread::show_login,this));
	url.add("^/logout$",	BIND(&Main_Thread::show_logout,this));
	url.add("^/newpost$",	BIND(&Main_Thread::show_post_form,this));
	url.add("^/post$",	BIND(&Main_Thread::get_post_message,this));
	url.add("^/dologin$",	BIND(&Main_Thread::do_login,this));
	url.add("^/edit/(\\d+)$",BIND(&Main_Thread::edit_message,this,$1));

	try {
		db.open();
	}
	catch(DB_Err &e)
	{
		throw HTTP_Error(e.get());
	}
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
	DB_Res res=db.query(
		escape( "SELECT password,id from cp_users "
			"WHERE username='%1%' LIMIT 1")<<name);
	
	DB_Row row;
	
	if((row=res.next())!=NULL){
		if(password==row[0]) {
			authenticated=true;
			user_id=atoi(row[1]);
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

void Main_Thread::text2html(char const *text,string &s)
{
	int c;
	s="";
	s.reserve(strlen(text)*3/2);
	
	while((c=*text++)!=0) {
		switch(c) {
			case '\n': s+="<br>\n"; break;
			case '<':  s+="&lt;"; break;
			case '>':  s+="&gt;"; break;
			case '&':  s+="&amp;"; break;
			default:
				s+=(char)c;
		}
	}
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
	
	DB_Res res;
	
	if(from=="end") {
		res = db.query(
			"SELECT cp_messages.id,message,username "
			"FROM cp_messages "
			"LEFT JOIN cp_users ON cp_messages.user_id=cp_users.id "
			"ORDER BY cp_messages.id DESC LIMIT 10");
	}
	else {
		int from_id=atoi(from.c_str());
		char const q[]=
			"SELECT cp_messages.id,message,username "
			"FROM cp_messages "
			"LEFT JOIN cp_users ON cp_messages.user_id=cp_users.id "
			"WHERE cp_messages.id < %1% "
			"ORDER BY cp_messages.id DESC LIMIT 10";

		res = db.query(escape(q)<<from_id);
	}
	
	DB_Row row;

	int id;
	string content;
	while((id=t.render(out))!=0) {
		if(id==TV_get_message){
			if((row=res.next())!=NULL){
				c[TV_new_message]=1;
				c[TV_message_id]=row[0];
				text2html(row[1],content);
				c[TV_message_body]=content.c_str();
				c[TV_author]=row[2];
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
	db.exec(escape("INSERT INTO cp_messages (message,user_id) values('%1%',%2%)")
			<<message<<user_id);
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
	catch (DB_Err &mysql_err) {
		throw HTTP_Error(mysql_err.get());
	}
}
