{
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script" : "/chat"
	},
	"file_server" : {
		"enable" : true,
		"alias" : [
			{ 
				"url" : "/scripts" ,
				"path" : "../../contrib/client_side/jsonrpc" 
			}
		],
		"document_root" : "."
	},

}

