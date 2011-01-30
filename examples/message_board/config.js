{
	"mb" : {
		"media" : "/media",
		"root" : "/mb",
		"connection_string" : "sqlite3:db=mb.db;@pool_size=10"
	},
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script" : "/mb" 
	},
	"views" : {
		 "paths" : [ "./" ],
		 "skins" : [ "simple" ],
	},
	"file_server" : {
		"enable" : true,
		"document_root" : "../"
	},
	"session" : {
		"expire" : "renew",
		"timeout" : 604800,
		"location" : "client",
		"client" : {
			"hmac" : "sha1",
			"hmac_key" : "232074faa0fd37de20858bf8cd0a7d04"
		}
	},
	"localization" : {
		"messages" : {
			"paths" : [ "./locale" ],
			"domains" : [ "mb" ]
		},
		"locales" : [ 
			"en_US.UTF-8",
			"he_IL.UTF-8"
		]
	}
}
