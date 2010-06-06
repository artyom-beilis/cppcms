{
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script_names" : [ "/mb" ]
	},
	"file_server" : {
		"enable" : true,
		"doument_root" : "."
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
	"localization" : {
		"encoding" : "utf-8",
		"messages" : {
			"paths" : [ "./locale" ],
			"domains" : [ "mb" ]
		},
		"locales" : [ "he_IL" ]
	},
	"mb" : {
		"media" : "/media"
	}
}
