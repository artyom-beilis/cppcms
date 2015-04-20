{
	"service" : {
		"api" : "http",
		"ip" : "0.0.0.0",
		"port" : 8080
	},
	"http" : {
		"script" : "/hello"
	},
	"file_server" : {
		"enable" : true,
		"document_root" : "./"
	},
	"session" : {
		"expire" : "renew",
		"timeout" : 604800,
		"location" : "client",
		"client" :      {
			"hmac" :        "sha1",
			"hmac_key" :    "3891bbf7f845fd4277008a63d72640fc13bb9a31"
		}

	},
}
