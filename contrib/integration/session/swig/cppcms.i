%{
#include <cppcms/capi/session.h>
%}

%constant SESSION_FIXED = 0;
%constant SESSION_RENEW = 1;
%constant SESSION_BROWSER = 2;

%constant ERROR_OK = 0;
%constant ERROR_GENERAL = 1;
%constant ERROR_RUNTIME = 2;
%constant ERROR_INVALID_ARGUMENT = 4;
%constant ERROR_LOGIC = 5;
%constant ERROR_ALLOC = 6;

%rename(CppCMSAPIPool) cppcms_capi_session_pool;
typedef struct cppcms_capi_session_pool{} cppcms_capi_session_pool;
%rename(CppCMSAPISession) cppcms_capi_session;
typedef struct cppcms_capi_session{} cppcms_capi_session;
%rename(CppCMSAPICookie) cppcms_capi_cookie;
typedef struct cppcms_capi_cookie{} cppcms_capi_cookie;

%extend cppcms_capi_session_pool {
        cppcms_capi_session_pool() { return cppcms_capi_session_pool_new(); }
        ~cppcms_capi_session_pool() { return cppcms_capi_session_pool_delete($self); }
        int error() { return cppcms_capi_error($self); }
        char const *error_message() { return cppcms_capi_error_message($self); }
        char const *error_clear() { return cppcms_capi_error_clear($self); }
        int init(char const *conf);
        int init_from_json(char const *conf); 
};

%extend cppcms_capi_session {
        cppcms_capi_session() { return cppcms_capi_session_new(); }
        ~cppcms_capi_session() { return cppcms_capi_session_delete($self); }
        int error() { return cppcms_capi_error($self); }
        char const *error_message() { return cppcms_capi_error_message($self); }
        char const *error_clear() { return cppcms_capi_error_clear($self); }

        int init(cppcms_capi_session_pool *pool);
        int clear();
        int is_set(char const *key);
        int erase(char const *key);
        int get_exposed(char const *key);
        int set_exposed(char const *key,int is_exposed);
        char const *get_first_key();
        char const *get_next_key();
        char const *get_csrf_token();
        int set(char const *key,char const *value);
        char const *get(char const *key);
        int set_binary_as_hex(char const *key,char const *value);
        char const *get_binary_as_hex(char const *key);
        #int set_binary(char const *key,void const *value,int length);
        #int get_binary(char const *key,void *buf,int buffer_size);
        int get_binary_len(char const *key);
        int reset_session();
        int set_default_age();
        int set_age(int t);
        int get_age();
        int set_default_expiration();
        int set_expiration(int t);
        int get_expiration();
        int set_on_server(int is_on_server);
        int get_on_server();
        char const *get_session_cookie_name();
        int load(char const *session_cookie_value);
        int save();
        %newobject cookie_first;
        cppcms_capi_cookie *cookie_first();
        %newobject cookie_next;
        cppcms_capi_cookie *cookie_next();
};

%extend cppcms_capi_cookie {
        cppcms_capi_cookie() { return NULL; }
        ~cppcms_capi_cookie() { cppcms_capi_cookie_delete($self); }
        char const *header();
        char const *header_content();
        char const *name();
        char const *value();
        char const *path();
        char const *domain();
        int max_age_defined();
        unsigned max_age();
        int expires_defined();
        long long expires();
        int is_secure();
};


