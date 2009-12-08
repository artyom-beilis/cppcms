// CppCMS Configuration file
// Extended JSON format, "//" like comments are allowed

{
	// Service description
	"service" : {
		"procs" : 0,
		"worker_threads": 5,
		"api" : "http",
		"port" : 8080,
	//	"ip" : "0.0.0.0"
		"ip" : "127.0.0.1"
		// "socket" : "/tmp/scgi.socket"
	},
	"http" : {
		"proxy" : {
			"behind" : true,
			"remote_addr_headers" : 
				[ 
			 		"X-Real-IP", // Nginx One
					"X-Forwarded-For" // Standard One
				]
		},
		"script_names" : [ "/stock", "/hello" , "/chat" ]
		//"script" : "/hello"
	},
	"gzip" : {
		"enable" : true, // Default true
		// "level" : 1,
		// "buffer" : 4096
	},
	"localization" : {
		"encoding" : "UTF-8",
		"messages" : {
			"paths" : [ "./transtext/locale" ],
			"domains" : [ "test", "app" ]
		},
		"locales" : [ "he_IL", "en_US", "he_IL@calendar=hebrew" ],
		"disable_charset_in_content_type" : false	// Disable 
	},
	"session" : {
		"expire" : "browser",
		"timeout" : 10,
		"cookies_prefix" : "cppcms_session",
		"cookies_domain" : "",
		"cookies_path" : "/",
		"cookies_secure" : false
	},
	"views" : {
		"default_skin" : "skin1",
		 "paths" : [ "./" ],
		 // "skins" : [ "skin3" ],
		 "auto_reload" : true
	},
	"file_server" : {
		"enable" : true,
		"doument_root" : "."
		// "mime_types" : "mime.type"
	}
	
}
