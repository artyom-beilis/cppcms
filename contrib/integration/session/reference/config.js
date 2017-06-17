{
	"service" : {
		"api" : "http",
		"port" : 8080,
		"ip" : "0.0.0.0"
	},
	"http" : {
		"script" : "/test"
	},
	"session" : {
		"expire" : "renew",
		"timeout" : 604800,
		"location" : "both",
		"client" :      {
			"hmac" :        "sha1",
			"hmac_key" :    "3891bbf7f845fd4277008a63d72640fc13bb9a31"
		},
		"server" : {
			"storage" : "memory"
		}

	},
	"security" : {  "csrf" : {"enable" : true}},
	"file_server" : {
		"enable" : true,
		"document_root" : "."
	},
}
