#include <cppcms/capi/session.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(x) do { if(!(x)) { printf("Error %s in %d\n",#x,__LINE__); exit(1); }} while(0)
#define CHECK(p) do{ if(cppcms_capi_error(p)!=0) { printf("Failed %s in %d\n",cppcms_capi_error_message(p),__LINE__); exit(1); }} while(0)

int main(int argc,char **argv)
{
	cppcms_capi_cookie *cookie=0;
	cppcms_capi_session_pool *session_pool=0;
	cppcms_capi_session *session = 0;
	char buf[5]={1,2,3,4,5};
	char *state=strdup("");
	int n=0;

	if(argc!=2) {
		fprintf(stderr,"usage external_session_test config.js\n");
		return 1;
	}

	printf("Create session\n");
	session_pool=cppcms_capi_session_pool_new();
	cppcms_capi_session_pool_init(session_pool,argv[1]);
	CHECK(session_pool);


	// State A
	session = cppcms_capi_session_new();
	cppcms_capi_session_init(session,session_pool);
	CHECK(session);

	cppcms_capi_session_set_session_cookie(session,state);
	cppcms_capi_session_load(session);
	CHECK(session);
	cppcms_capi_session_set(session,"x","test");
	cppcms_capi_session_set(session,"yyy","111");
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

	cppcms_capi_session_set_session_cookie(session,state);
	cppcms_capi_session_add_cookie_name(session,"sc_tt");
	cppcms_capi_session_load(session);
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
	TEST(cppcms_capi_session_set_binary(session,"yyy","\0\xFA\0\1",4)==0);
	TEST(strcmp(cppcms_capi_session_get_binary_as_hex(session,"yyy"),"00fa0001")==0);


	cppcms_capi_session_set_exposed(session,"x",0);
	cppcms_capi_session_save(session);
	CHECK(session);
	n=0;
	for(cookie=cppcms_capi_session_cookie_first(session);cookie;cookie=cppcms_capi_session_cookie_next(session)) {
		printf("   %s\n",cppcms_capi_cookie_header(cookie));
		if(strcmp(cppcms_capi_cookie_name(cookie),cppcms_capi_session_get_session_cookie_name(session))==0) {
			free(state);
			state = strdup(cppcms_capi_cookie_value(cookie));
			n += 1;
		}
		else if(strcmp(cppcms_capi_cookie_name(cookie),"sc_x")==0) {
			TEST(strcmp(cppcms_capi_cookie_value(cookie),"")==0);
			TEST(cppcms_capi_cookie_max_age_defined(cookie)==1);
			TEST(cppcms_capi_cookie_max_age(cookie)==0);
			n += 10;
		}
		else if(strcmp(cppcms_capi_cookie_name(cookie),"sc_tt")==0) {
			TEST(strcmp(cppcms_capi_cookie_value(cookie),"")==0);
			TEST(cppcms_capi_cookie_max_age_defined(cookie)==1);
			TEST(cppcms_capi_cookie_max_age(cookie)==0);
			n += 100;
		}
		else {
			n+=1000;
		}
		cppcms_capi_cookie_delete(cookie);
	}
	TEST(n==111);
	
	cppcms_capi_session_delete(session);
	printf("New state=[%s]\n",state);

	// State C
	session = cppcms_capi_session_new();
	cppcms_capi_session_init(session,session_pool);
	CHECK(session);

	cppcms_capi_session_set_session_cookie(session,state);
	cppcms_capi_session_load(session);
	CHECK(session);

	TEST(cppcms_capi_session_get_exposed(session,"x")==0);
	TEST(cppcms_capi_session_get_exposed(session,"yyy")==0);
	TEST(cppcms_capi_session_is_set(session,"x"));
	TEST(cppcms_capi_session_is_set(session,"yyy"));
	TEST(cppcms_capi_session_is_set(session,"zz")==0);
	TEST(strcmp(cppcms_capi_session_get(session,"x"),"test")==0);
	TEST(memcmp(cppcms_capi_session_get(session,"yyy"),"\0\xfa\0\1",4)==0);
	TEST(cppcms_capi_session_get_binary_len(session,"yyy")==4);
	TEST(cppcms_capi_session_get_binary(session,"yyy",buf,sizeof(buf))==4);
	TEST(memcmp(buf,"\0\xFA\0\1\5",5)==0);
	TEST(cppcms_capi_session_set_binary_as_hex(session,"yyy","DEADbeef")==0);
	TEST(cppcms_capi_session_get_binary(session,"yyy",buf,4)==4);
	TEST(memcmp(buf,"\xde\xad\xbe\xef\5",5)==0);


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
	
	TEST(cppcms_capi_session_get(session,NULL)==0);
	TEST(cppcms_capi_error(session)==CPPCMS_CAPI_ERROR_INVALID_ARGUMENT);
	TEST(strstr(cppcms_capi_error_message(session),"null")!=0);
	TEST(strstr(cppcms_capi_error_clear(session),"null")!=0);
	TEST(strcmp(cppcms_capi_error_message(session),"ok")==0);
	TEST(cppcms_capi_error(session)==CPPCMS_CAPI_ERROR_OK);
	TEST(cppcms_capi_session_save(session)==-1);
	TEST(cppcms_capi_error(session)==CPPCMS_CAPI_ERROR_LOGIC);


	cppcms_capi_session_delete(session);
	printf("New state=[%s]\n",state);

	///////////////////

	cppcms_capi_session_pool_delete(session_pool);
	free(state);
	printf("Ok\n");
	return 0;
}
