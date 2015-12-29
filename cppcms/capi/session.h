///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2012  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  See accompanying file COPYING.TXT file for licensing details.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CPPCMS_CAPI_SESSION_H
#define CPPCMS_CAPI_SESSION_H
#include <cppcms/defs.h>

#ifdef __cplusplus
extern "C" {
#endif
///
/// \defgroup capi_session CppCMS C Session API 
/// @{
/// \ver{v1_2}
///

/// \defgroup capi_session_expiration Expiration Codes
/// @{
#define CPPCMS_CAPI_SESSION_FIXED	0 ///< Fixed session expiration 
#define CPPCMS_CAPI_SESSION_RENEW	1 ///< Session expiration automatically renewed on access 
#define CPPCMS_CAPI_SESSION_BROWSER	2 ///< Session is tied to browser session
/// @}

///
/// \defgroup capi_session_error_handling Error Handling
/// @{
#define CPPCMS_CAPI_ERROR_OK		0 ///< Success 
#define CPPCMS_CAPI_ERROR_GENERAL	1 ///< Uncategorized error
#define CPPCMS_CAPI_ERROR_RUNTIME	2 ///< Runtime Error occured
#define CPPCMS_CAPI_ERROR_INVALID_ARGUMENT	4 ///< Invalid Argument Provided
#define CPPCMS_CAPI_ERROR_LOGIC		5 ///< Invalid API Use
#define CPPCMS_CAPI_ERROR_ALLOC		6 ///< Allocation Failed

///
/// "Base class" for cppcms_capi_session, cppcms_capi_session_pool, and cppcms_capi_cookie
///
/// Is used to get error status of the object
///
typedef void *cppcms_capi_object; 

/// @}

/// Class that represents cppcms::session_pool in C
///
/// \defgroup cppcms_capi_session_pool Session Pool
typedef struct cppcms_capi_session_pool cppcms_capi_session_pool;
/// Class that represents cppcms::session_interface in C
///
/// \defgroup cppcms_capi_session_pool_init_from_json Session Interface
typedef struct cppcms_capi_session cppcms_capi_session;
/// Class that represents cppcms::http::cookie in C
///
/// \defgroup cppcms_capi_session_cookie Cookie
typedef struct cppcms_capi_cookie cppcms_capi_cookie;

///
/// \addtogroup capi_session_error_handling 
/// @{

CPPCMS_API int cppcms_capi_error(cppcms_capi_object obj); ///< Get error code for the object
CPPCMS_API char const *cppcms_capi_error_message(cppcms_capi_object obj); ///< Get error message for the object
CPPCMS_API char const *cppcms_capi_error_clear(cppcms_capi_object obj); ///< Clear error and get last message

/// @}

/// \addtogroup cppcms_capi_session_pool
/// @{

/// Create new empty pool object, returns NULL in case of fatal error
CPPCMS_API cppcms_capi_session_pool *cppcms_capi_session_pool_new();
/// Destroys pool object
CPPCMS_API void cppcms_capi_session_pool_delete(cppcms_capi_session_pool *pool);

/// Initialize the pool from CppCMS configuration file,\a config_file
CPPCMS_API int cppcms_capi_session_pool_init(cppcms_capi_session_pool *pool,char const *config_file);
/// Initialize the pool from CppCMS configuration in json fromat \a json
CPPCMS_API int cppcms_capi_session_pool_init_from_json(cppcms_capi_session_pool *pool,char const *json);

/// @}

CPPCMS_API cppcms_capi_session *cppcms_capi_session_new();
CPPCMS_API void cppcms_capi_session_delete(cppcms_capi_session *session);


CPPCMS_API int cppcms_capi_session_init(cppcms_capi_session *session,cppcms_capi_session_pool *pool);

CPPCMS_API int cppcms_capi_session_clear(cppcms_capi_session *session);

CPPCMS_API int cppcms_capi_session_is_set(cppcms_capi_session *session,char const *key);
CPPCMS_API int cppcms_capi_session_erase(cppcms_capi_session *session,char const *key);
CPPCMS_API int cppcms_capi_session_get_exposed(cppcms_capi_session *session,char const *key);
CPPCMS_API int cppcms_capi_session_set_exposed(cppcms_capi_session *session,char const *key,int is_exposed);

CPPCMS_API char const *cppcms_capi_session_get_first_key(cppcms_capi_session *session);
CPPCMS_API char const *cppcms_capi_session_get_next_key(cppcms_capi_session *session);


CPPCMS_API char const *cppcms_capi_session_get_csrf_token(cppcms_capi_session *session);


CPPCMS_API int cppcms_capi_session_set(cppcms_capi_session *session,char const *key,char const *value);
CPPCMS_API char const *cppcms_capi_session_get(cppcms_capi_session *session,char const *key);

CPPCMS_API int cppcms_capi_session_set_binary_as_hex(cppcms_capi_session *session,char const *key,char const *value);
CPPCMS_API char const *cppcms_capi_session_get_binary_as_hex(cppcms_capi_session *session,char const *key);

CPPCMS_API int cppcms_capi_session_set_binary(cppcms_capi_session *session,char const *key,void const *value,int length);
CPPCMS_API int cppcms_capi_session_get_binary(cppcms_capi_session *session,char const *key,void *buf,int buffer_size);
CPPCMS_API int cppcms_capi_session_get_binary_len(cppcms_capi_session *session,char const *key);


CPPCMS_API int cppcms_capi_session_reset_session(cppcms_capi_session *session);

CPPCMS_API int cppcms_capi_session_set_default_age(cppcms_capi_session *session);
CPPCMS_API int cppcms_capi_session_set_age(cppcms_capi_session *session,int t);
CPPCMS_API int cppcms_capi_session_get_age(cppcms_capi_session *session);

CPPCMS_API int cppcms_capi_session_set_default_expiration(cppcms_capi_session *session);
/// Define expiration method
CPPCMS_API int cppcms_capi_session_set_expiration(cppcms_capi_session *session,int t);
/// Get current expiration method
CPPCMS_API int cppcms_capi_session_get_expiration(cppcms_capi_session *session);

CPPCMS_API int cppcms_capi_session_set_on_server(cppcms_capi_session *session,int is_on_server);
CPPCMS_API int cppcms_capi_session_get_on_server(cppcms_capi_session *session);

CPPCMS_API char const *cppcms_capi_session_get_session_cookie_name(cppcms_capi_session *session);
CPPCMS_API int cppcms_capi_session_set_session_cookie(cppcms_capi_session *session,char const *session_cookie_value);
CPPCMS_API int cppcms_capi_session_add_cookie_name(cppcms_capi_session *session,char const *name);
CPPCMS_API int cppcms_capi_session_load(cppcms_capi_session *session);
CPPCMS_API int cppcms_capi_session_save(cppcms_capi_session *session);

CPPCMS_API cppcms_capi_cookie *cppcms_capi_session_cookie_first(cppcms_capi_session *session);
CPPCMS_API cppcms_capi_cookie *cppcms_capi_session_cookie_next(cppcms_capi_session *session);

CPPCMS_API void cppcms_capi_cookie_delete(cppcms_capi_cookie *cookie);

CPPCMS_API char const *cppcms_capi_cookie_header(cppcms_capi_cookie const *cookie);
CPPCMS_API char const *cppcms_capi_cookie_header_content(cppcms_capi_cookie const *cookie);

CPPCMS_API char const *cppcms_capi_cookie_name(cppcms_capi_cookie const *cookie);
CPPCMS_API char const *cppcms_capi_cookie_value(cppcms_capi_cookie const *cookie);
CPPCMS_API char const *cppcms_capi_cookie_path(cppcms_capi_cookie const *cookie);
CPPCMS_API char const *cppcms_capi_cookie_domain(cppcms_capi_cookie const *cookie);

CPPCMS_API int cppcms_capi_cookie_max_age_defined(cppcms_capi_cookie const *cookie);
CPPCMS_API unsigned cppcms_capi_cookie_max_age(cppcms_capi_cookie const *cookie);

CPPCMS_API int cppcms_capi_cookie_expires_defined(cppcms_capi_cookie const *cookie);
CPPCMS_API long long cppcms_capi_cookie_expires(cppcms_capi_cookie const *cookie);

CPPCMS_API int cppcms_capi_cookie_is_secure(cppcms_capi_cookie const *cookie);

///
/// @}
///
#ifdef __cplusplus
} // extern "C"
#endif
#endif
