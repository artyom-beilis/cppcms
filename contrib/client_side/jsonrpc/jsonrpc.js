//
//  Copyright (c) 2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

///
/// Create new JsonRPC service.
///
/// Constructor:
///   Parameters:
///   - uri - string - service URI
///   - function_methods - optional array of strings - method names that return results
///   - notification_methods - optional array of strings - notification method names
///
/// Useful methods:
///
/// addRPCMethod(name,id)
///    Creates new method with named name
///    Parameters:
///    - name - new method name
///    - id - JSONRPC id. It should be null for notification methods
///       and it should be some integer or string for function methods
///
function JsonRPC(uri,function_methods,notification_methods) {
	this.uri = uri;
	if(typeof function_methods != 'undefined') {
		for(var i=0;i<function_methods.length;i++)
		{
			this.addRPCMethod(function_methods[i],i);
		}
	}
	if(typeof notification_methods != 'undefined') {
		for(var i=0;i<notification_methods.length;i++) 
		{
			this.addRPCMethod(notification_methods[i],null);
		}
	}
}

JsonRPC.prototype.getXHR = function() {
	// Create new XMLHttpRequest for all browsers
	var xhr;
	if(typeof  XMLHttpRequest != "undefined") {
		xhr = new XMLHttpRequest();
	}
	else {
		try { xhr = new ActiveXObject("Msxml2.XMLHTTP.6.0"); }catch(e){}
		try { xhr = new ActiveXObject("Msxml2.XMLHTTP.3.0"); }catch(e){}
		try { xhr = new ActiveXObject("Microsoft.XMLHTTP"); }catch(e){}
		throw new Error("No XML HTTP Rewquest support");
	}
	return xhr;
}

JsonRPC.prototype.runXHR = function(xhr,name,id,params) {
	// Perform actual XML HTTP Request
	xhr.setRequestHeader("Content-Type","application/json");
	var request = {'id' : id,'method' : name, 'params' : params };
	var requestText = JSON.stringify(request);
	xhr.send(requestText);
}

JsonRPC.prototype.syncCall = function(name,id,params) {
	// Synchronous method call name = method name
	// id null for notifcation something for functions
	// params - array of parameters
	var xhr = this.getXHR();
	xhr.open("post",this.uri,false);
	this.runXHR(xhr,name,id,params);
	if(xhr.status!=200) 
		throw Error('Invalid response:' + xhr.status);
	if(id!=null) {
		var response = JSON.parse(xhr.responseText);
		if(response.error != null) 
			throw Error(response.error);
		return response.result;
	}
}

JsonRPC.prototype.asyncCall = function(name,id,params,on_result,on_error) {
	// Asynchronous method call name = method name
	// id null for notifcation something for functions
	// params - array of parameters
	// on_result and on_error - the callbacks
	//
	var xhr = this.getXHR();
	xhr.open("post",this.uri,true);
	xhr.onreadystatechange = function() {
		if(xhr.readyState!=4)
			return;
		if(xhr.status==200) {
			if(id!=null) {
				var response = JSON.parse(xhr.responseText);
				if(response.error != null) {
					on_error({'type': 'response', 'error' : response.error });
				}
				else {
					on_result(response.result);
				}
			}
			else {
				on_result();
			}
		}
		else {
			on_error( { 'type' : 'transport', 'error' : xhr.status } );
		}
	}
	this.runXHR(xhr,name,id,params);
}
///
/// Add new method, specify id = null for notification other valid id for function
///
JsonRPC.prototype.addRPCMethod = function(method,id) {
	var call = function() {
		var args = new Array();
		for(var i=0;i<arguments.length;i++) {
			args[i]=arguments[i];
		}
		if(call.on_result != null) {
			this.asyncCall(method,id,args,call.on_result,call.on_error);
		}
		else
			return this.syncCall(method,id,args);
	};
	call.on_error = function(e) {
		throw Error(e.error);
	}
	call.on_result = null;
	this[method]=call;
}

