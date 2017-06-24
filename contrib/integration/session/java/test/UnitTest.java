import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import com.cppcms.session.*;

public class UnitTest extends HttpServlet {
	static SessionPool pool;
	
	public void init() throws ServletException 
	{
		ServletContext context = getServletContext();
		String configPath = context.getRealPath("/WEB-INF/config.js");
		pool = SessionPool.openFromConfig(configPath);
	}

	public static String toHex(byte[] data)
	{
		StringBuilder bld = new StringBuilder();
		for(int i=0;i<data.length;i++) {
			int v=data[i] & 0xFF;
			bld.append(String.format("%02x",v));
		}
		return bld.toString();
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
	public static byte[] fromHex(String value)
	{
		byte[] values = new byte[value.length() / 2];
		for(int i=0;i<values.length;i++) {
			int v = c2v(value.charAt(2*i)) * 16 + c2v(value.charAt(2*i+1));
			values[i] = (byte)(v & 0xFF);
		}
		return values;
	}

	public void doGet(HttpServletRequest request,
			HttpServletResponse response) 
		throws ServletException, IOException 
	{
		Session session = null;
		try {
			session = pool.getSession();
			session.load(request);
			StringBuilder final_result = new StringBuilder();
			
			for(int i=1;;i++) {
				String id = "_" + Integer.toString(i);
				String op = request.getParameter("op" + id);
				if(op == null)
					break;
				String key = request.getParameter("key" + id);
				String value = request.getParameter("value" + id);
				String result = "ok";

				switch(op) {
				case "is_set":
					result = session.isSet(key) ? "yes" : "no";
					break;
				case "erase":
					session.erase(key);
					break;
				case "clear":
					session.clear();
					break;
				case "is_exposed":
					result = session.getExposed(key) ? "yes" : "no";
					break;
				case  "expose":
					session.setExposed(key,Integer.parseInt(value)!=0);
					break;
				case "get":
					result = session.get(key);
					break;
				case "set":
					session.set(key,value);
					break;
				case "get_binary":
					result = toHex(session.getBinary(key));
					break;
				case "set_binary":
					session.setBinary(key,fromHex(value));
					break;
				case "get_age":
					result = Integer.toString(session.getAge());
					break;
				case "set_age":
					session.setAge(Integer.parseInt(value));
					break;
				case "default_age":
					session.setDefaultAge();
					break;
				case "get_expiration":
					result = Integer.toString(session.getExpiration());
					break;
				case "set_expiration":
					session.setExpiration(Integer.parseInt(value));
					break;
				case "default_expiration":
					session.setDefaultExpiration();
					break;
				case "get_on_server":
					result = session.getOnServer() ? "yes" : "no";
					break;
				case "set_on_server":
					session.setOnServer(Integer.parseInt(value) != 0);
					break;
				case "reset_session":
					session.resetSession();
					break;
				case "csrf_token":
					result = "t=" + session.getCSRFToken();
					break;
				case "keys":
					result = "";
					String[] keys = session.getKeys();
					for(int j=0;j<keys.length;j++) {
						if(!result.isEmpty())
							result += ",";
						result += "[" + keys[j] + "]";
					}
					break;
				default:
					result = "invalid op=" + op;
				}
				final_result.append(Integer.toString(i)  + ":"  + result +  ";");
			}

			session.save(response);

			PrintWriter out = response.getWriter();
			out.write(final_result.toString());
		}
		finally {
			if(session != null)
				session.close();
		}
		// Use "out" to send content to browser
	}
}

