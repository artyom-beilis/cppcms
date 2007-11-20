#include "main_thread.h"

#include "global_config.h"

void Main_Thread::init()
{
	Worker_Thread::init();
	
	try {
		db.open();
	}
	catch(MySQL_DB_Err &e)
	{
		throw HTTP_Error(e.get());
	}
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

void Main_Thread::check_athentication()
{

	MySQL_DB_Res res=db.query(
		escape( "SELECT password,id from cp_users "
			"WHERE username='%1%' LIMIT 1")<<username);
	
	MySQL_DB_Row row;
	
	if((row=res.next())!=NULL){
		if(password==row[0]) {
			authenticated=true;
			user_id=atoi(row[1]);
		}
	}
	else {
		username.clear();
		authenticated=false;
	}
}

void Main_Thread::load_inputs()
{
	
	const vector<FormEntry> &elements=cgi->getElements();
	unsigned i;	
	
	page=0;
	message.clear();

	page=url.parse();	
	
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
	out.puts(
	"<html><body>"
	"<form name=\"input\" action=\"/site/dologin\" method=\"post\">\n"
	" Username: <input type=\"text\" name=\"username\">\n<br>"
	" Password: <input type=\"password\" name=\"password\">\n"
	//"<input type=\"hidden\" name=\"id\" value=\"dologin\">\n"
	" <input type=\"submit\" value=\"Submit\"></form>\n"
	"</html></body>");
}

void Main_Thread::do_login()
{
	username=new_username;
	password=new_password;
	check_athentication();
	if(authenticated) {
		set_header(new HTTPRedirectHeader("/site/"));
		HTTPCookie cookie("username",username);
		cookie.setMaxAge(7*24*3600);
		response_header->setCookie(cookie);
		cookie.setName("password");
		cookie.setValue(password);
		response_header->setCookie(cookie);
	}
	else {
		set_header(new HTTPRedirectHeader("/site/login"));
	}
};

void Main_Thread::show_logout()
{
	set_header(new HTTPRedirectHeader("/site/"));
	response_header->setCookie(HTTPCookie("username",""));
	response_header->setCookie(HTTPCookie("password",""));
}

void Main_Thread::printhtml(char const *text)
{
	int c;
	while((c=*text++)!=0) {
		switch(c) {
			case '\n': out.puts("<br>\n"); break;
			case '<': out.puts("&lt;"); break;
			case '>': out.puts("&gt;"); break;
			case '&': out.puts("&amp;"); break;
			default:
				out.putchar(c);
		}
	}
}

void Main_Thread::show_main_page()
{
	if(authenticated) {
		out.printf("<h1>Wellcome %s to forum</h1>\n",username.c_str());
	}
	else {
		out.puts("<h1>Wellcome to the forum</h1>");
	}
	out.puts("<a href=\"/site/newpost\">New Post</a><br>");
	out.puts("<a href=\"/site/logout\">Logout</a><br>");
	MySQL_DB_Res res = db.query(
		"SELECT cp_messages.id,message,username "
		"FROM cp_messages,cp_users "
		"WHERE cp_messages.user_id=cp_users.id "
		"ORDER BY cp_messages.id DESC LIMIT 10");
	
	MySQL_DB_Row row;
	out.puts("<dl>\n");
	while((row=res.next())!=NULL) {
		out.printf("<dt>Message %s, by %s</dt>\n<dd>\n",row[0],row[2]);
		printhtml(row[1]);
		out.puts("</dd>\n");
	}
	out.puts("</dl>\n");
}

void Main_Thread::show_post_form()
{
	if(authenticated) {
		out.puts(
		"<html><body>"
		"<form name=\"input\" action=\"/site/post\" method=\"post\">\n"
		"<TEXTAREA NAME=\"message\" COLS=40 ROWS=6></TEXTAREA><br>\n"
		//"<input type=\"hidden\" name=\"id\" value=\"post\">\n"
		" <input type=\"submit\" value=\"Submit\"></form>\n"
		"</html></body>");
	}
	else {
		set_header(new HTTPRedirectHeader("/site/login"));
		
	}
}

void Main_Thread::get_post_message()
{
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
	switch(page) {
		case 1: show_login(); break;
		case 4: if(authenticated) 
				get_post_message();
			else
				show_login();
			break;
		case 3: if(authenticated)
				show_post_form();
			else
				show_login();
			break;
		case 2: show_logout(); break;
		case 5: do_login(); break;
		case 0:
		default:
			show_main_page();
	}
}
void Main_Thread::main()
{
	try {
		set_header(new HTTPHTMLHeader);
		
		load_cookies();
		check_athentication();
		load_inputs();
		show_page();
	}
	catch (MySQL_DB_Err &mysql_err) {
		throw HTTP_Error(mysql_err.get());
	}
}
