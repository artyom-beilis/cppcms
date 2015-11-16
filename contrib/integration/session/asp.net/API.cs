using System;
using System.Text;
using System.Runtime.InteropServices;
using System.Web;

namespace CppCMS.Session  {
internal class API {
        public const int SESSION_FIXED=0;
        public const int SESSION_RENEW=1;
        public const int SESSION_BROWSER=2;
        public const int ERROR_OK=0;
        public const int ERROR_GENERAL=1;
        public const int ERROR_RUNTIME=2;
        public const int ERROR_INVALID_ARGUMENT=4;
        public const int ERROR_LOGIC=5;
        public const int ERROR_ALLOC=6;

	public const string library = "cppcms";

        [DllImport(library,EntryPoint="cppcms_capi_error")]
        public static extern int error(IntPtr obj);

        [DllImport(library,EntryPoint="cppcms_capi_error_message")]
        public static extern IntPtr error_message(IntPtr obj);

        [DllImport(library,EntryPoint="cppcms_capi_error_clear")]
        public static extern IntPtr error_clear(IntPtr obj);

        [DllImport(library,EntryPoint="cppcms_capi_session_pool_new")]
        public static extern IntPtr session_pool_new();

        [DllImport(library,EntryPoint="cppcms_capi_session_pool_delete")]
        public static extern void session_pool_delete(IntPtr pool);

        [DllImport(library,EntryPoint="cppcms_capi_session_pool_init")]
        public static extern int session_pool_init(IntPtr pool,byte[]  config_file);

        [DllImport(library,EntryPoint="cppcms_capi_session_pool_init_from_json")]
        public static extern int session_pool_init_from_json(IntPtr pool,byte[]  json);

        [DllImport(library,EntryPoint="cppcms_capi_session_new")]
        public static extern IntPtr session_new();

        [DllImport(library,EntryPoint="cppcms_capi_session_delete")]
        public static extern void session_delete(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_init")]
        public static extern int session_init(IntPtr session,IntPtr pool);

        [DllImport(library,EntryPoint="cppcms_capi_session_clear")]
        public static extern int session_clear(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_is_set")]
        public static extern int session_is_set(IntPtr session,byte[]  key);

        [DllImport(library,EntryPoint="cppcms_capi_session_erase")]
        public static extern int session_erase(IntPtr session,byte[]  key);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_exposed")]
        public static extern int session_get_exposed(IntPtr session,byte[]  key);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_exposed")]
        public static extern int session_set_exposed(IntPtr session,byte[]  key,int is_exposed);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_first_key")]
        public static extern IntPtr session_get_first_key(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_next_key")]
        public static extern IntPtr session_get_next_key(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_csrf_token")]
        public static extern IntPtr session_get_csrf_token(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_set")]
        public static extern int session_set(IntPtr session,byte[]  key,byte[]  val);

        [DllImport(library,EntryPoint="cppcms_capi_session_get")]
        public static extern IntPtr session_get(IntPtr session,byte[]  key);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_binary_as_hex")]
        public static extern int session_set_binary_as_hex(IntPtr session,byte[]  key,byte[]  val);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_binary_as_hex")]
        public static extern IntPtr session_get_binary_as_hex(IntPtr session,byte[]  key);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_binary")]
        public static extern int session_set_binary(IntPtr session,byte[]  key,byte[] val,int length);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_binary")]
        public static extern int session_get_binary(IntPtr session,byte[]  key,byte[] buf,int buffer_size);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_binary_len")]
        public static extern int session_get_binary_len(IntPtr session,byte[]  key);

        [DllImport(library,EntryPoint="cppcms_capi_session_reset_session")]
        public static extern int session_reset_session(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_default_age")]
        public static extern int session_set_default_age(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_age")]
        public static extern int session_set_age(IntPtr session,int t);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_age")]
        public static extern int session_get_age(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_default_expiration")]
        public static extern int session_set_default_expiration(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_expiration")]
        public static extern int session_set_expiration(IntPtr session,int t);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_expiration")]
        public static extern int session_get_expiration(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_set_on_server")]
        public static extern int session_set_on_server(IntPtr session,int is_on_server);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_on_server")]
        public static extern int session_get_on_server(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_get_session_cookie_name")]
        public static extern IntPtr session_get_session_cookie_name(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_load")]
        public static extern int session_load(IntPtr session,byte[]  session_cookie_value);

        [DllImport(library,EntryPoint="cppcms_capi_session_save")]
        public static extern int session_save(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_cookie_first")]
        public static extern IntPtr session_cookie_first(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_session_cookie_next")]
        public static extern IntPtr session_cookie_next(IntPtr session);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_delete")]
        public static extern void cookie_delete(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_header")]
        public static extern IntPtr cookie_header(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_header_content")]
        public static extern IntPtr cookie_header_content(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_name")]
        public static extern IntPtr cookie_name(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_value")]
        public static extern IntPtr cookie_value(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_path")]
        public static extern IntPtr cookie_path(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_domain")]
        public static extern IntPtr cookie_domain(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_max_age_defined")]
        public static extern int cookie_max_age_defined(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_max_age")]
        public static extern uint cookie_max_age(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_expires_defined")]
        public static extern int cookie_expires_defined(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_expires")]
        public static extern long cookie_expires(IntPtr cookie);

        [DllImport(library,EntryPoint="cppcms_capi_cookie_is_secure")]
        public static extern int cookie_is_secure(IntPtr cookie);

	public static void Main(string[] args)
	{
		using(SessionPool pool = SessionPool.FromConfig(args[0])) {
			string state = "";
			using(Session session = pool.Session()) {
				session.Load(state);
				session["x"]="y";
				session["y"]="124";
				session.SetExposed("y",true);
				foreach(string k in session.Keys) {
					Console.WriteLine("Got Key " + k);
				}
				session.Save();
				foreach(Cookie c in session.Cookies) {
					if(c.Name == session.SessionCookieName)
						state = c.Value;
					Console.WriteLine("Got Cookie " + c);
				}
			}
			using(Session session = pool.Session()) {
				session.Load(state);
				session.SetExposed("y",false);
				session.Save();
				foreach(Cookie c in session.Cookies) {
					if(c.Name == session.SessionCookieName)
						state = c.Value;
					Console.WriteLine("Got Cookie " + c);
				}
				foreach(HttpCookie c in session.HttpCookies) {
					Console.WriteLine("Got HttpCookie " + c.Name +  " expires at " + c.Expires );
				}
			}
		}
		Console.WriteLine("Ok");
	}

} // class
} // namespace
