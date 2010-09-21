{
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script" : "/hello"
	},
	"session" : {
		"expire" : "renew",
		"timeout" : 604800,
		"location" : "client",
		"client" : {
			"encryptor" : "hmac",
			"key" : "232074faa0fd37de20858bf8cd0a7d04"
		}
	},
}
