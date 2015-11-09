#include <cppcms/capi/session.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(x) do { if(!(x)) { printf("Error %s in %d\n",#x,__LINE__); exit(1); }} while(0)
#define CHECK(p) do{ if(cppcms_capi_##p##_got_error(p)!=0) { printf("Failed %s in %d\n",cppcms_capi_##p##_strerror(p),__LINE__); exit(1); }} while(0)

int main(int argc,char **argv)
{
	cppcms_capi_cookie *cookie=0;
	cppcms_capi_session_pool *session_pool=0;
	cppcms_capi_session *session = 0;
	char *state=strdup("");
	int n=0;

	printf("Create session\n");
	session_pool=cppcms_capi_session_pool_new();
	cppcms_capi_session_pool_init(session_pool,argv[1]);
	CHECK(session_pool);


	// State A
	session = cppcms_capi_session_new();
	cppcms_capi_session_init(session,session_pool);
	CHECK(session);

	cppcms_capi_session_load(session,state);
	CHECK(session);
	cppcms_capi_session_set(session,"x","test",-1);
	cppcms_capi_session_set(session,"yyy","111",-1);
	cppcms_capi_session_set_exposed(session,"x",1);
	printf("Check keys first time\n");
	TEST(strcmp(cppcms_capi_session_get_first_key(session),"x")==0);
	TEST(strcmp(cppcms_capi_session_get_next_key(session),"yyy")==0);
	TEST(cppcms_capi_session_get_next_key(session)==0);
	TEST(strcmp(cppcms_capi_session_get_first_key(session),"x")==0);
	TEST(cppcms_capi_session_get_exposed(session,"x")==1);
	TEST(cppcms_capi_session_get_exposed(session,"yyy")==0);
	cppcms_capi_session_save(session);

	CHECK(session);
	n=0;
	for(cookie=cppcms_capi_session_cookie_first(session);cookie;cookie=cppcms_capi_session_cookie_next(session)) {
		printf("   %s\n",cppcms_capi_cookie_header(cookie));
		n++;
		if(strcmp(cppcms_capi_cookie_name(cookie),cppcms_capi_session_get_session_cookie_name(session))==0) {
			free(state);
			state = strdup(cppcms_capi_cookie_value(cookie));
		}
		else if(strcmp(cppcms_capi_cookie_name(cookie),"sc_x")==0) {
			TEST(strcmp(cppcms_capi_cookie_value(cookie),"test")==0);
		}
		cppcms_capi_cookie_delete(cookie);
	}
	TEST(n==2);
	cppcms_capi_session_delete(session);
	printf("New state=[%s]\n",state);

	// State B
	session = cppcms_capi_session_new();
	cppcms_capi_session_init(session,session_pool);
	CHECK(session);

	cppcms_capi_session_load(session,state);
	CHECK(session);
	
	printf("Check keys second time\n");
	TEST(strcmp(cppcms_capi_session_get_first_key(session),"x")==0);
	TEST(strcmp(cppcms_capi_session_get_next_key(session),"yyy")==0);
	TEST(cppcms_capi_session_get_next_key(session)==0);
	TEST(strcmp(cppcms_capi_session_get_first_key(session),"x")==0);
	TEST(cppcms_capi_session_get_exposed(session,"x")==1);
	TEST(cppcms_capi_session_get_exposed(session,"yyy")==0);
	TEST(cppcms_capi_session_is_set(session,"x"));
	TEST(cppcms_capi_session_is_set(session,"yyy"));
	TEST(cppcms_capi_session_is_set(session,"zz")==0);
	TEST(strcmp(cppcms_capi_session_get(session,"x"),"test")==0);
	TEST(strcmp(cppcms_capi_session_get(session,"yyy"),"111")==0);
	TEST(cppcms_capi_session_set(session,"yyy","\0\0\0\1",4)==0);


	cppcms_capi_session_set_exposed(session,"x",0);
	cppcms_capi_session_save(session);
	CHECK(session);
	n=0;
	for(cookie=cppcms_capi_session_cookie_first(session);cookie;cookie=cppcms_capi_session_cookie_next(session)) {
		printf("   %s\n",cppcms_capi_cookie_header(cookie));
		n++;
		if(strcmp(cppcms_capi_cookie_name(cookie),cppcms_capi_session_get_session_cookie_name(session))==0) {
			free(state);
			state = strdup(cppcms_capi_cookie_value(cookie));
		}
		else if(strcmp(cppcms_capi_cookie_name(cookie),"sc_x")==0) {
			TEST(strcmp(cppcms_capi_cookie_value(cookie),"")==0);
			TEST(cppcms_capi_cookie_max_age_defined(cookie)==1);
			TEST(cppcms_capi_cookie_max_age(cookie)==0);
		}
		cppcms_capi_cookie_delete(cookie);
	}
	TEST(n==2);
	
	cppcms_capi_session_delete(session);
	printf("New state=[%s]\n",state);

	// State C
	session = cppcms_capi_session_new();
	cppcms_capi_session_init(session,session_pool);
	CHECK(session);

	cppcms_capi_session_load(session,state);
	CHECK(session);

	TEST(cppcms_capi_session_get_exposed(session,"x")==0);
	TEST(cppcms_capi_session_get_exposed(session,"yyy")==0);
	TEST(cppcms_capi_session_is_set(session,"x"));
	TEST(cppcms_capi_session_is_set(session,"yyy"));
	TEST(cppcms_capi_session_is_set(session,"zz")==0);
	TEST(strcmp(cppcms_capi_session_get(session,"x"),"test")==0);
	TEST(memcmp(cppcms_capi_session_get(session,"yyy"),"\0\0\0\1",4)==0);
	TEST(cppcms_capi_session_get_len(session,"yyy")==4);


	cppcms_capi_session_clear(session);
	cppcms_capi_session_save(session);
	CHECK(session);
	n=0;
	for(cookie=cppcms_capi_session_cookie_first(session);cookie;cookie=cppcms_capi_session_cookie_next(session)) {
		printf("   %s\n",cppcms_capi_cookie_header(cookie));
		n++;
		if(strcmp(cppcms_capi_cookie_name(cookie),cppcms_capi_session_get_session_cookie_name(session))==0) {
			TEST(strcmp(cppcms_capi_cookie_value(cookie),"")==0);
			TEST(cppcms_capi_cookie_max_age_defined(cookie)==1);
			TEST(cppcms_capi_cookie_max_age(cookie)==0);
			free(state);
			state = strdup(cppcms_capi_cookie_value(cookie));
		}
		cppcms_capi_cookie_delete(cookie);
	}
	TEST(n==1);
	
	cppcms_capi_session_delete(session);
	printf("New state=[%s]\n",state);

	///////////////////

	cppcms_capi_session_pool_delete(session_pool);
	free(state);
	printf("Ok\n");
	return 0;
}
