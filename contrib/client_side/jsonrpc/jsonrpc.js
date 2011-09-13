function JsonRPC(uri,function_methods,notification_methods) {
	this.uri = uri;

	// Create new XMLHttpRequest for all browsers
	this.getXHR = function() {
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
	};

	// Synchronous method call name = method name
	// id null for notifcation something for functions
	// params - array of parameters
	this.syncCall = function(name,id,params) {
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
	};
	this.runXHR = function(xhr,name,id,params) {
		xhr.setRequestHeader("Content-Type","application/json");
		var request = {'id' : id,'method' : name, 'params' : params };
		var requestText = JSON.stringify(request);
		xhr.send(requestText);
	};
	this.asyncCall = function(name,id,params,on_result,on_error) {
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
	this.addRPCMethod = function(method,id) {
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

