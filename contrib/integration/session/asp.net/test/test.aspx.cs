using System;
using System.Text;
using System.Web.UI;
using CppCMS.Session;

public partial class UnitTest : Page {
	static SessionPool pool;
	static UnitTest() {
		pool = SessionPool.FromConfig("../../reference/config.js");
	}
	protected string message = null;

	public static string toHex(byte[] data)
	{
        string parts="0123456789abcdef";
		StringBuilder bld = new StringBuilder();
		for(int i=0;i<data.Length;i++) {
			int v=data[i] & 0xFF;
			bld.Append(parts[v >> 4]);
			bld.Append(parts[v & 0xF]);
		}
		return bld.ToString();
	}
	static int c2v(char c)
	{
		if('0' <=c && c<='9')
			return (int)c-'0';
		if('a' <=c && c<='f')
			return (int)(c)-(int)('a')+10;
		if('A' <=c && c<='F')
			return (int)(c)-(int)('A')+10;
		return 0;
	}
	public static byte[] fromHex(string val)
	{
		byte[] values = new byte[val.Length / 2];
		for(int i=0;i<values.Length;i++) {
			int v = c2v(val[2*i]) * 16 + c2v(val[2*i+1]);
			values[i] = (byte)(v & 0xFF);
		}
		return values;
	}

    static string int2str(int v)
    {
        return v.ToString();
    }


	protected void Page_Load(object sender,EventArgs e)
	{
		using(Session session = pool.Session()) {
			session.Load(Request);
			
            StringBuilder final_result = new StringBuilder();
			
			for(int i=1;;i++) {
				string id = "_" + int2str(i);
				string op = Request.QueryString["op" + id];
				if(op == null)
					break;
				string key = Request.QueryString["key" + id];
				string val = Request.QueryString["value" + id];
				string result = "ok";

				switch(op) {
				case "is_set":
					result = session.IsSet(key) ? "yes" : "no";
					break;
				case "erase":
					session.Erase(key);
					break;
				case "clear":
					session.Clear();
					break;
				case "is_exposed":
					result = session.GetExposed(key) ? "yes" : "no";
					break;
				case  "expose":
					session.SetExposed(key,Int32.Parse(val)!=0);
					break;
				case "get":
					result = session[key];
					break;
				case "set":
					session[key]=val;
					break;
				case "get_binary":
					result = toHex(session.GetBinary(key));
					break;
				case "set_binary":
					session.SetBinary(key,fromHex(val));
					break;
				case "get_age":
					result = int2str(session.Age);
					break;
				case "set_age":
					session.Age = Int32.Parse(val);
					break;
				case "default_age":
					session.SetDefaultAge();
					break;
				case "get_expiration":
					result = int2str(session.Expiration);
					break;
				case "set_expiration":
					session.Expiration = Int32.Parse(val);
					break;
				case "default_expiration":
					session.SetDefaultExpiration();
					break;
				case "get_on_server":
					result = session.OnServer ? "yes" : "no";
					break;
				case "set_on_server":
					session.OnServer = Int32.Parse(val) != 0;
					break;
				case "reset_session":
					session.ResetSession();
					break;
				case "csrf_token":
					result = "t=" + session.CSRFToken;
					break;
				case "keys":
					result = "";
					string[] keys = session.Keys;
					for(int j=0;j<keys.Length;j++) {
						if(result != "")
							result += ",";
						result += "[" + keys[j] + "]";
					}
					break;
				default:
					result = "invalid op=" + op;
                    break;
				}
				final_result.Append(int2str(i)  + ":"  + result +  ";");
			}

			session.Save(Response);
			message = final_result.ToString();
		}
	}
}
