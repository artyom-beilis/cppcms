using System;
using System.Web.UI;
using CppCMS.Session;
public partial class Example : Page {
	static SessionPool pool;
	static Example() {
		pool = SessionPool.FromConfig("cppcms-config.js");
	}
	protected string counter = null;
	protected void Page_Load(object sender,EventArgs e)
	{
		using(Session s = pool.Session()) {
			s.Load(Request);
			string v="0";
			if(s.IsSet("x"))
				v=s["x"];
			v = (int.Parse(v) + 1).ToString();
			s["x"]=v;
			s.Save(Response);
			counter = v;
		}
	}
}
