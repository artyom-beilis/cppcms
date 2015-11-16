using System;
namespace CppCMS.Session  {
public class SessionPool : SessionBase {
	private SessionPool()
	{
		d=API.session_pool_new();
	}
	override public void Close()
	{
		if(d!=IntPtr.Zero) {
			API.session_pool_delete(d);
			d=IntPtr.Zero;
		}
	}
	public static SessionPool FromConfig(string path)
	{
		SessionPool p = new SessionPool();
		API.session_pool_init(p.d,tb(path));
		p.check();
		return p;
	}
	public static SessionPool FromJSON(string json)
	{
		SessionPool p = new SessionPool();
		API.session_pool_init_from_json(p.d,tb(json));
		p.check();
		return p;
	}
	public Session Session()
	{
		return new Session(this);
	}
}
} // namespace

