{
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script" : "/rpc"
	},
	"file_server" : { 
		"enable" : true,
		"alias" : [
			{ 
				"url" : "/scripts" ,
				"path" : "../../contrib/client_side/jsonrpc" 
			}
		]
	}
}

