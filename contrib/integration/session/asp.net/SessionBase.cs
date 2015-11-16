using System;
using System.Text;
using System.Runtime.InteropServices;

namespace CppCMS.Session  {

abstract public class SessionBase : IDisposable {

	internal IntPtr d = IntPtr.Zero;
	
	abstract public void Close();

	public void Dispose()
	{
		Close();
	}
	~SessionBase()
	{
		Close();
	}

	internal static byte[] tb(string s)
	{
		if(s==null)
			return null;
		return Encoding.UTF8.GetBytes(s + "\0");
	}
	internal static string ts(IntPtr p)
	{
		if(p==IntPtr.Zero)
			return null;
		int len = 0;
		while(Marshal.ReadByte(p,len)!=0)
			len++;
		byte[] tmp = new byte[len];
		Marshal.Copy(p,tmp,0,len);
		return Encoding.UTF8.GetString(tmp);
	}
	protected void check()
	{
		int code = API.error(d);
		if(code == 0)
			return;
		string msg = ts(API.error_clear(d));
		switch(code) {
		case API.ERROR_ALLOC:
			throw new OutOfMemoryException(msg);
		case API.ERROR_INVALID_ARGUMENT:
			throw new ArgumentException(msg);
		case API.ERROR_LOGIC:
			throw new InvalidOperationException(msg);
		default:
			throw new SystemException(msg);
		}
	}
} // class
} // namespace
