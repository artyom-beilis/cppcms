function JsonRPC(uri,methods) {
	this.uri = uri;
	this.getXHR = function() {
		if(typeof  XMLHttpRequest != "undefined")
			return new XMLHttpRequest();
		try { return new ActiveXObject("Msxml2.XMLHTTP.6.0"); }catch(e){}
		try { return new ActiveXObject("Msxml2.XMLHTTP.3.0"); }catch(e){}
		try { return new ActiveXObject("Microsoft.XMLHTTP"); }catch(e){}
		throw new Error("No XML HTTP Rewquest support");
	};
	this.syncCall = function(name,params) {
		xhr = this.getXHR();
		xhr.open("post",this.uri,false);
		this.runXHR(xhr,name,params);
		if(xhr.status!=200) 
			throw Error('Invalid response:' + xhr.status);
		response = JSON.parse(xhr.responseText);
		if(response.error != null) {
			throw Error(response.error);
		}
		return response.result;
	};
	this.runXHR = function(xhr,name,params) {
		xhr.setRequestHeader("Content-Type","application/json");
		request = {'id' : 1,'method' : name, 'params' : params };
		requestText = JSON.stringify(request);
		xhr.send(requestText);

	};
	this.asyncCall = function(name,params,on_result,on_error) {
		xhr = this.getXHR();
		xhr.open("post",this.uri,true);
		xhr.onreadystatechange = function() {
			if(xhr.readyState!=4)
				return;
			if(xhr.status==200) {
				response = JSON.parse(xhr.responseText);
				if(response.error == null)
					on_result(response.result);
				else
					on_error({'type': 'response', 'error' : response.error });
			}
			else {
				on_error( { 'type' : 'transport', 'error' : xhr.status } );
			}
		}
		this.runXHR(xhr,name,params);
	}
	this.addRPCMethod = function(method) {
		var call = function() {
			args = new Array();
			for(var i=0;i<arguments.length;i++) {
				args[i]=arguments[i];
			}
			if(call.on_result != null) {
				this.asyncCall(method,args,call.on_result,call.on_error);
			}
			else
				return this.syncCall(method,args);
		};
		call.on_error = function(e) {
			throw Error(e.error);
		}
		call.on_result = null;
		this[method]=call;
	}
	for(i=0;i<methods.length;i++)
	{
		this.addRPCMethod(methods[i]);
	}
}

