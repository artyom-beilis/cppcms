package com.cppcms.session;

import com.sun.jna.Pointer;


public class CppCMSCookie {

	public final String name;
	public final String value;
	public final String path;
	public final String domain;
	public final String header;
	public final String headerContent;
	public final boolean maxAgeDefined;
	public final int maxAge;
	public final boolean expiresDefined;
	public final long expires;
	public final boolean isSecure;

	public String toString()
	{
		return header;
	}

	protected CppCMSCookie(Pointer p)
	{
		try {
			this.name = API.api.cppcms_capi_cookie_name(p);
			this.value = API.api.cppcms_capi_cookie_value(p);
			this.path = API.api.cppcms_capi_cookie_path(p);
			this.domain = API.api.cppcms_capi_cookie_domain(p);
			this.header = API.api.cppcms_capi_cookie_header(p);
			this.headerContent = API.api.cppcms_capi_cookie_header_content(p);
			this.maxAgeDefined = API.api.cppcms_capi_cookie_max_age_defined(p)!=0;
			this.expiresDefined = API.api.cppcms_capi_cookie_expires_defined(p)!=0;
			this.maxAge = maxAgeDefined ? API.api.cppcms_capi_cookie_max_age(p) : -1;
			this.expires = expiresDefined ? API.api.cppcms_capi_cookie_expires(p) : -1;
			this.isSecure = API.api.cppcms_capi_cookie_is_secure(p)!=0;
		}
		finally {
			API.api.cppcms_capi_cookie_delete(p);
		}
	}
}
