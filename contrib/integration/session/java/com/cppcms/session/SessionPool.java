package com.cppcms.session;

import com.sun.jna.Pointer;


public class SessionPool extends SessionBase {

	 private SessionPool()
	 {
		API.init();
		d=API.api.cppcms_capi_session_pool_new();
	 }
	 public static SessionPool openFromConfig(String file) 
	 {
		 SessionPool p=new SessionPool();
		 API.api.cppcms_capi_session_pool_init(p.d,file);
		 p.check();
		 return p;
	 }
	 public static SessionPool openFromJson(String json)
	 {
		 SessionPool p=new SessionPool();
		 API.api.cppcms_capi_session_pool_init_from_json(p.d,json);
		 p.check();
		 return p;
	 }
	 public Session getSession()
	 {
		 return new Session(d);
	 }
	 void doClose()
	 {
		API.api.cppcms_capi_session_pool_delete(d);
	 }
};
