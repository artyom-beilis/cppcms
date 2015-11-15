package com.cppcms.session;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;

public class API {
	
	final static int ERROR_OK=0;
        final static int ERROR_GENERAL=1;
        final static int ERROR_RUNTIME=2;
        final static int ERROR_INVALID_ARGUMENT=4;
        final static int ERROR_LOGIC=5;
        final static int ERROR_ALLOC=6;

	protected interface JnaAPI extends Library {
		int cppcms_capi_error(Pointer obj);
		String cppcms_capi_error_message(Pointer obj);
		String cppcms_capi_error_clear(Pointer obj);
		Pointer cppcms_capi_session_pool_new();
		void cppcms_capi_session_pool_delete(Pointer pool);
		int cppcms_capi_session_pool_init(Pointer pool,String config_file);
		int cppcms_capi_session_pool_init_from_json(Pointer pool,String json);

		Pointer cppcms_capi_session_new();
		void cppcms_capi_session_delete(Pointer session);
		int cppcms_capi_session_init(Pointer session,Pointer pool);
		int cppcms_capi_session_clear(Pointer session);
		int cppcms_capi_session_is_set(Pointer session,String key);
		int cppcms_capi_session_erase(Pointer session,String key);
		int cppcms_capi_session_get_exposed(Pointer session,String key);
		int cppcms_capi_session_set_exposed(Pointer session,String key,int is_exposed);
		String cppcms_capi_session_get_first_key(Pointer session);
		String cppcms_capi_session_get_next_key(Pointer session);
		String cppcms_capi_session_get_csrf_token(Pointer session);
		int cppcms_capi_session_set(Pointer session,String key,String value);
		String cppcms_capi_session_get(Pointer session,String key);
		int cppcms_capi_session_set_binary_as_hex(Pointer session,String key,String value);
		String cppcms_capi_session_get_binary_as_hex(Pointer session,String key);
		int cppcms_capi_session_set_binary(Pointer session,String key,byte[] value,int length);
		int cppcms_capi_session_get_binary(Pointer session,String key,byte[] buf,int buffer_size);
		int cppcms_capi_session_get_binary_len(Pointer session,String key);
		int cppcms_capi_session_reset_session(Pointer session);
		int cppcms_capi_session_set_default_age(Pointer session);
		int cppcms_capi_session_set_age(Pointer session,int t);
		int cppcms_capi_session_get_age(Pointer session);
		int cppcms_capi_session_set_default_expiration(Pointer session);
		int cppcms_capi_session_set_expiration(Pointer session,int t);
		int cppcms_capi_session_get_expiration(Pointer session);
		int cppcms_capi_session_set_on_server(Pointer session,int is_on_server);
		int cppcms_capi_session_get_on_server(Pointer session);
		String cppcms_capi_session_get_session_cookie_name(Pointer session);
		int cppcms_capi_session_load(Pointer session,String session_cookie_value);
		int cppcms_capi_session_save(Pointer session);
		Pointer cppcms_capi_session_cookie_first(Pointer session);
		Pointer cppcms_capi_session_cookie_next(Pointer session);
		void cppcms_capi_cookie_delete(Pointer cookie);
		String cppcms_capi_cookie_header(Pointer cookie);
		String cppcms_capi_cookie_header_content(Pointer cookie);
		String cppcms_capi_cookie_name(Pointer cookie);
		String cppcms_capi_cookie_value(Pointer cookie);
		String cppcms_capi_cookie_path(Pointer cookie);
		String cppcms_capi_cookie_domain(Pointer cookie);
		int cppcms_capi_cookie_max_age_defined(Pointer cookie);
		int cppcms_capi_cookie_max_age(Pointer cookie);
		int cppcms_capi_cookie_expires_defined(Pointer cookie);
		long cppcms_capi_cookie_expires(Pointer cookie);
		int cppcms_capi_cookie_is_secure(Pointer cookie);	
	}

	protected static JnaAPI api;

	private static void loadLibrary(String path)
	{
		api = (JnaAPI)Native.loadLibrary(path,JnaAPI.class);	
	}
	public static synchronized void init(String path)
	{
		if(api == null) {
			loadLibrary(path);
		}
	}
	public static synchronized void init()
	{
		if(api != null)
			return;

		String names[]=null;
		String os=System.getProperty("os.name").toLowerCase();
		if(os.indexOf("win")!=-1)
			names = new String[] { "cppcms.dll", "libcppcms.dll", "cppcms-1.dll", "libcppcms-1.dll" };
		else if(os.indexOf("mac")!=-1)
			names=new String[] { "libcppcms.dylib", "libcppcms.1.dylib" };
		else 
			names=new String[] { "libcppcms.so", "libcppcms.so.1" };
		for(int i=0;i<names.length;i++) {
			try {
				loadLibrary(names[i]);
			}
			catch(Exception e) {}
			catch(Error e){}
		}
		if(api==null) {
			StringBuilder sb = new StringBuilder();
			sb.append("Failed to load any of the following:");
			for(int i=0;i<names.length;i++) {
				sb.append(" ");
				sb.append(names[i]);
			}
			throw new RuntimeException(sb.toString()); 
		}
	}

	public static void main(String[] args) 
	{
		String path=null;
		String conf=null;
		if(args.length == 2) {
			path = args[0];
			conf = args[1];
		}
		else if(args.length == 1) {
			conf = args[0];
		} 
		else {
			System.out.println("usage [/path/to/libcppcms.so] /path/to/config.js");
			return;
		}
		SessionPool p = null;
		Session s = null;
		try {
			String state="";
			if(path!=null)
				init(path);
			p = SessionPool.openFromConfig(conf);
			s = p.getSession();
			s.load(state);
			s.set("x","1");
			s.setExposed("x",true);
			s.save();
			CppCMSCookie[] cookies=s.getCookies();
			for(CppCMSCookie c: cookies) {
				System.out.println(c);
				if(c.name.equals(s.getSessionCookieName()))
					state = c.value;
			}
			s.close();
			s=p.getSession();
			s.load(state);
			System.out.println("Value of x=" + s.get("x"));
			byte[] v=new byte[1];
			v[0]=65;
			s.setBinary("y",v);
			s.save();
			cookies=s.getCookies();
			for(CppCMSCookie c: cookies) {
				System.out.println(c);
				if(c.name.equals(s.getSessionCookieName()))
					state = c.value;
			}
			s.close();
			s = p.getSession();
			s.load(state);
			byte[] v2=s.getBinary("y");
			System.out.println("v2.len=" + v2.length + " v[0]=" + v[0]);
			s.set("x","2222");
			s.save();
			javax.servlet.http.Cookie[] cs=s.getServletCookies();
			for(javax.servlet.http.Cookie c : cs) {
				System.out.println("Got=" + c.getValue());
			}
			s.close();

		}
		finally {
			if(s!=null)
				s.close();
			if(p!=null)
				p.close();

		}

	}
}

