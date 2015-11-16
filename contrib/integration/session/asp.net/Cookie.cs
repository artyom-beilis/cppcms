using System;
using System.Runtime.InteropServices;

namespace CppCMS.Session  {
public class Cookie  {

	public readonly string Header;
	public readonly string Name;
	public readonly string Value;
	public readonly string Domain;
	public readonly string Path;
	public readonly string HeaderContent;
	public readonly bool MaxAgeDefined;
	public readonly bool ExpiresDefined;
	public readonly bool Secure;
	public readonly uint MaxAge;
	public readonly long Expires;

	private static String ts(IntPtr p)
	{
		return SessionBase.ts(p);
	}
	internal Cookie(IntPtr d) 
	{
		Header = ts(API.cookie_header(d)); 
		Name = ts(API.cookie_name(d)); 
		Value = ts(API.cookie_value(d)); 
		Domain = ts(API.cookie_domain(d)); 
		Path = ts(API.cookie_path(d)); 
		HeaderContent = ts(API.cookie_header_content(d)); 
		MaxAgeDefined = API.cookie_max_age_defined(d)!=0; 
		ExpiresDefined = API.cookie_expires_defined(d)!=0; 
		Secure = API.cookie_is_secure(d)!=0; 
		MaxAge = API.cookie_max_age(d); 
		Expires = API.cookie_expires(d); 
	}
	public override string ToString()
	{
		return this.Header;
	}
} // class
} // namespcae

