import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import com.cppcms.session.*;

public class Sample extends HttpServlet {
	static SessionPool pool;
	
	public void init() throws ServletException 
	{
		ServletContext context = getServletContext();
		String configPath = context.getRealPath("/WEB-INF/cppcms-config.js");
		pool = SessionPool.openFromConfig(configPath);
	}

	public void doGet(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException 
	{
		Session session = pool.getSession();
		session.load(request);
		String x=session.get("x");
		if(x==null) {
			x="0";
		}
		x=Integer.toString(Integer.parseInt(x)+1);
		session.set("x",x);
		session.save(response);
		session.close();

		PrintWriter out = response.getWriter();
		out.println("X is now " + x);
		// Use "out" to send content to browser
	}
}

