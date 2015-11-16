using System;
using System.Collections;
using System.Runtime.InteropServices;
using System.Web;

namespace CppCMS.Session  {
public class Session : SessionBase {
        public const int SESSION_FIXED=0;
        public const int SESSION_RENEW=1;
        public const int SESSION_BROWSER=2;

	internal Session(SessionPool pool)
	{
		d=API.session_new();
		API.session_init(d,pool.d);
		try {
			check();
		}
		catch(Exception) {
			Close();
			throw;
		}
	}
	override public void Close()
	{
		if(d!=IntPtr.Zero) {
			API.session_delete(d);
			d=IntPtr.Zero;
		}
	}
	public void Clear() { API.session_clear(d); check(); }
	public bool IsSet(string key) { bool r = API.session_is_set(d,tb(key))!=0; check(); return r; }
	public void Erase(string key) { API.session_erase(d,tb(key)); check(); }
	public bool GetExposed(string key) { bool r = API.session_get_exposed(d,tb(key))!=0; check(); return r; }
	public void SetExposed(string key,bool v) { API.session_set_exposed(d,tb(key),v?1:0); check(); }
	public string[] Keys {
		get {
			ArrayList a=new ArrayList();
			string s=ts(API.session_get_first_key(d));
			while(s!=null) {
				a.Add(s);
				s=ts(API.session_get_next_key(d));
			}
			check();
			return (string[])a.ToArray(typeof(string));
		}
	}
	public string CSRFToken { get { string r=ts(API.session_get_csrf_token(d)); check(); return r; } }
	public void Set(string key,string val) { API.session_set(d,tb(key),tb(val)); check(); }
	public string Get(string key) { string r = ts(API.session_get(d,tb(key))); check(); return r; }
	public string this[string key] {
		get { return Get(key); }
		set { Set(key,value); }
	}
	public void SetBinary(string key,byte[] val) { API.session_set_binary(d,tb(key),val,val.Length); check(); }
	public byte[] GetBinary(string key) { 
		byte[] bkey = tb(key);
		int len = API.session_get_binary_len(d,bkey);
		check();
		byte[] res = new byte[len];
		API.session_get_binary(d,bkey,res,len);
		check();
		return res;
	}
	public void ResetSession() { API.session_reset_session(d); check(); }
	public void SetDefaultAge() { API.session_set_default_age(d); check(); }
	public int Age {
		set { API.session_set_age(d,value); check(); }
		get { int r = API.session_get_age(d); check(); return r; }
	}

	public void SetDefaultExpiration() { API.session_set_default_expiration(d); check(); }
	public int Expiration {
		set { API.session_set_expiration(d,value); check(); }
		get { int r = API.session_get_expiration(d); check(); return r; }
	}
	public bool OnServer {
		set { API.session_set_on_server(d,value ? 1 : 0); check(); }
		get { bool r = API.session_get_on_server(d)!=0; check(); return r; }
	}
	public string SessionCookieName {
		get { string r=ts(API.session_get_session_cookie_name(d)); check(); return r; }
	}
	
	public void Load(String cookie) { API.session_load(d,tb(cookie)); check(); }
	public void Load(HttpRequest r)
	{
		HttpCookie c=r.Cookies[SessionCookieName];
		if(c==null)
			Load("");
		else
			Load(c.Value);
	}
	public void Save() { API.session_save(d); check(); }
	public Cookie[] Cookies {
		get {
			ArrayList a=new ArrayList();
			IntPtr cookie = API.session_cookie_first(d);
			while(cookie!=IntPtr.Zero) {
				a.Add(new Cookie(cookie));
				API.cookie_delete(cookie);
				cookie = API.session_cookie_next(d);
			}
			check();
			return (Cookie[])a.ToArray(typeof(Cookie));
		}
	}
	public HttpCookie[] HttpCookies {
		get {
			Cookie[] ccs = Cookies;
			HttpCookie[] hcs = new HttpCookie[ccs.Length];
			for(int i=0;i<ccs.Length;i++) {
				Cookie cc =ccs[i];
				HttpCookie hc = new HttpCookie(cc.Name,cc.Value);
				if(cc.Path!="")
					hc.Path = cc.Path;
				if(cc.Domain!="")
					hc.Domain = cc.Domain;
				hc.Secure = cc.Secure;
				if(cc.MaxAgeDefined || cc.ExpiresDefined) {
					DateTime dt;
					if(cc.ExpiresDefined) {
						dt = new DateTime(1970,1,1,0,0,0,DateTimeKind.Utc);
						dt = dt.AddSeconds(cc.Expires);
					}
					else {
						if(cc.MaxAge == 0)
							dt = new DateTime(1970,1,1,0,0,1,DateTimeKind.Utc);
						else 
							dt = DateTime.UtcNow.AddSeconds(cc.MaxAge);
					}
					hc.Expires = dt;
				}
				hcs[i]=hc;
			}
			return hcs;
		}
	}
	public void Save(HttpResponse r)
	{
		Save();
		IntPtr cookie = IntPtr.Zero;
		try {
			cookie = API.session_cookie_first(d);
			while(cookie!=IntPtr.Zero) {
				string val = ts(API.cookie_header_content(cookie));
				API.cookie_delete(cookie);
				cookie=IntPtr.Zero;
				r.AppendHeader("Set-Cookie",val);
				cookie = API.session_cookie_next(d);
			}
			check();
		}
		finally {
			if(cookie != IntPtr.Zero)
				API.cookie_delete(cookie);
		}
	}


} // class
} // namespace
