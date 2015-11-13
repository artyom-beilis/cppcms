package com.cppcms.session;
import com.sun.jna.Pointer;

abstract class SessionBase {

	protected Pointer d;
	protected void finalize()
	{
		close();
	}
	protected void check()
	{
		if(d==null)
			throw new NullPointerException("Invalid class use");
		int code=API.api.cppcms_capi_error(d);
		if(code == 0)
			return;
		String msg = API.api.cppcms_capi_error_clear(d);
		switch(code) {
		case API.ERROR_ALLOC:
			throw new OutOfMemoryError(msg);
		case API.ERROR_INVALID_ARGUMENT:
			throw new IllegalArgumentException(msg);
		case API.ERROR_LOGIC:
			throw new IllegalStateException(msg);
		default:
			throw new RuntimeException(msg);
		}
	}

	abstract void doClose();
	
	public void close()
	{
		if(d!=null && API.api != null) {
			doClose();
			d=null;
		}
	}


};
