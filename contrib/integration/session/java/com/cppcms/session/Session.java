package com.cppcms.session;

import com.sun.jna.Pointer;
import java.util.ArrayList;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


public class Session extends SessionBase{
	
	public final static int EXPIRATION_FIXED=0;
	public final static int EXPIRATION_RENEW=1;
	public final static int EXPIRATION_BROWSER=2;

	protected Session(Pointer poolPtr) {
		try {
			d=API.api.cppcms_capi_session_new();
			API.api.cppcms_capi_session_init(d,poolPtr);
			check();
		}
		catch(RuntimeException e) {
			close();
			throw e;
		}
	}
	void doClose()
	{
		API.api.cppcms_capi_session_delete(d);
	}
	public void clear() {
		API.api.cppcms_capi_session_clear(d);
		check();
	}
	public boolean isSet(String key)
	{
		boolean r = API.api.cppcms_capi_session_is_set(d,key) != 0;
		check();
		return r;
	}
	public void erase(String key)
	{
		API.api.cppcms_capi_session_erase(d,key);
		check();
	}
	public boolean getExposed(String key)
	{
		boolean r=API.api.cppcms_capi_session_get_exposed(d,key)!=0;
		check();
		return r;
	}
	public void setExposed(String key,boolean exposed)
	{
		API.api.cppcms_capi_session_set_exposed(d,key,exposed?1:0);
		check();
	}
	public String[] getKeys()
	{
		ArrayList<String> lst=new ArrayList<String>();
		String s = API.api.cppcms_capi_session_get_first_key(d);
		while(s!=null) {
			lst.add(s);
			s=API.api.cppcms_capi_session_get_next_key(d);
		}
		check();
		return lst.toArray(new String[lst.size()]);
	}
	public String getCSRFToken()
	{
		String r = API.api.cppcms_capi_session_get_csrf_token(d);
		check();
		return r;
	}
	public void set(String key,String value)
	{
		API.api.cppcms_capi_session_set(d,key,value);
		check();
	}
	public String get(String key)
	{
		String r = API.api.cppcms_capi_session_get(d,key);
		check();
		return r;
	}
	public void setBinary(String key,byte[] data)
	{
		API.api.cppcms_capi_session_set_binary(d,key,data,data.length);
		check();
	}
	public byte[] getBinary(String key)
	{
		if(!isSet(key))
			return null;
		int len=API.api.cppcms_capi_session_get_binary_len(d,key);
		check();
		byte[] r = new byte[len];
		len=API.api.cppcms_capi_session_get_binary(d,key,r,len);
		check();
		return r;
	}
	public void resetSession()
	{
		API.api.cppcms_capi_session_reset_session(d);
		check();
	}
	public void setDefaultAge()
	{
		API.api.cppcms_capi_session_set_default_age(d);
		check();
	}
	public void setDefaultExpiration()
	{
		API.api.cppcms_capi_session_set_default_expiration(d);
		check();
	}
	public int getAge()
	{
		int r =API.api.cppcms_capi_session_get_age(d);
		check();
		return r;
	}
	public void setAge(int age)
	{
		API.api.cppcms_capi_session_set_age(d,age);
		check();
	}
	public int getExpiration()
	{
		int r =API.api.cppcms_capi_session_get_expiration(d);
		check();
		return r;
	}
	public void setExpiration(int exp)
	{
		API.api.cppcms_capi_session_set_expiration(d,exp);
		check();
	}
	public boolean getOnServer()
	{
		boolean r = API.api.cppcms_capi_session_get_on_server(d) != 0;
		check();
		return r;
	}
	public void setOnServer(boolean onServer)
	{
		API.api.cppcms_capi_session_set_on_server(d,onServer?1:0);
		check();
	}
	public String getSessionCookieName()
	{
		String r = API.api.cppcms_capi_session_get_session_cookie_name(d);
		check();
		return r;
	}
	public void load(String cookieValue)
	{
		API.api.cppcms_capi_session_load(d,cookieValue);
		check();
	}
	public void save()
	{
		API.api.cppcms_capi_session_save(d);
		check();
	}
	public Cookie[] getServletCookies()
	{
		CppCMSCookie[] ccs = getCookies();
		Cookie[] scs = new Cookie[ccs.length];
		for(int i=0;i<ccs.length;i++) {
			CppCMSCookie cs=ccs[i];
			Cookie c=new Cookie(cs.name,cs.value);
			if(!cs.path.isEmpty())
				c.setPath(cs.path);
			if(!cs.domain.isEmpty())
				c.setDomain(cs.domain);
			if(cs.maxAgeDefined || cs.expiresDefined) {
				if(cs.maxAgeDefined) {
					c.setMaxAge(cs.maxAge);
				}
				else {
					long diff = cs.expires - System.currentTimeMillis() / 1000;
					if(diff <= 0)
						c.setMaxAge(0);
					else
						c.setMaxAge((int)diff);
				}
			}
			c.setSecure(cs.isSecure);
			c.setVersion(1);
			scs[i]=c;
		}
		return scs;
	}
	public CppCMSCookie[] getCookies()
	{
		ArrayList<CppCMSCookie> cookies = new ArrayList<CppCMSCookie>();
		Pointer p = API.api.cppcms_capi_session_cookie_first(d);
		while(p!=null) {
			cookies.add(new CppCMSCookie(p));
			p = API.api.cppcms_capi_session_cookie_next(d);
		}
		check();
		return cookies.toArray(new CppCMSCookie[cookies.size()]);
	}
	public String[] getCookieHeaders()
	{
		ArrayList<String> cookies = new ArrayList<String>();
		Pointer p = API.api.cppcms_capi_session_cookie_first(d);
		while(p!=null) {
			cookies.add(API.api.cppcms_capi_cookie_header_content(p));
			API.api.cppcms_capi_cookie_delete(p);
			p = API.api.cppcms_capi_session_cookie_next(d);
		}
		check();
		return cookies.toArray(new String[cookies.size()]);
	}
	public void load(HttpServletRequest request)
	{
		String cookieName = getSessionCookieName();
		Cookie[] cookies = request.getCookies();
		for(int i=0;i<cookies.length;i++) {
			if(cookies[i].getName().equals(cookieName)) {
				load(cookies[i].getValue());
				return;
			}
		}
		load("");
	}
	public void save(HttpServletResponse response)
	{
		save();
		String[] cookies = getCookieHeaders();
		for(int i=0;i<cookies.length;i++) {
			response.addHeader("Set-Cookie",cookies[i]);
		}
	}

}
