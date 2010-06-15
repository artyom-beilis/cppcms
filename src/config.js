// CppCMS Configuration file
// Extended JSON format, "//" like comments are allowed

{
	"id" : 1,
	// Service description
	"service" : {
		//"worker_processes" : 5,
		"worker_threads" : 5,
		//"worker_threads" : 25,
	//	"api" : "fastcgi",
		"api" : "http",
		"port" : 8080,
	//	"port" : 8081,
		"ip" : "0.0.0.0",
	//	"ip" : "127.0.0.1",
	//	 "socket" : "/tmp/scgi.socket",
	//	"disable_global_exit_handling" , false
	//	"reactor" : "default" , // "select", "poll", "epoll", "devpoll", "kqueue"
		"nodes" : [
			{
				"id" : 1,
				"ip" : "127.0.0.1",
				"control_ports" : [ 8031 ] // one per forked process
			},
			{
				"id" : 2,
				"ip" : "127.0.0.1",
				"control_ports" : [ 8032 ]  //
			}
		]
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
		//"enable" : false, // Default true
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
		"timeout" : 10, // seconds
		"cookies" : {
			"prefix" : "cppcms_session",
			"domain" : "",
			"path" : "/",
			"secure" : false
		},
		//"location" : "client",
	//	"location" : "server",
	//	"location" : "both",
		"client_size_limit" : 1000,
		"gc" : 10,
		"client" : { 
			"encryptor" : "hmac", 
			//"encryptor" : "aes", 
			// aes or hmac -- hmac -- signature only, aes -- encryption and signature
			"key" : "261965ba80a79c034c9ae366a19a2627"
			// 32 digit hexadecimal secret number
		},
		"server" : {
			"storage" : "files",
			//"storage" : "memory",
			"dir" : "./cppcms_sessions",
			"shared" : true
		}
		
	},
	"views" : {
		"default_skin" : "skin1",
		 "paths" : [ "./" ],
		 //"skins" : [ "skin3" ],
		 //"auto_reload" : true
	},
	"cache" : {
		"backend" : "thread_shared", 
		//"backend" : "process_shared",
		"limit" : 100, // items - thread cache
		"memory" : 1024,  // KBs - process cache
//		"tcp" : {"ips" : [ "127.0.0.1" ],"ports" : [ 6001 ]}
	},
	"file_server" : {
		"enable" : true,
		"doument_root" : "."
		// "mime_types" : "mime.type"
	},
	"logging" : {
		//"level" : "info", // "debug",
		"level" : "error", // "debug",
		"stderr" : true,
		"file" : {
			"name" : "", // "cppcms.log" ,
			"max_files" : 3,
			"append" : true
		},
		"syslog" : {
			"enable" : false, //true,
			"id" : "cppcms",
			"options" : [ "LOG_CONS" ]
		}
	},
	"forwarding" : {
		"rules" : [
			//{
			//	"host" : "www.google.com",
			//	"script_name" : ".*\\.php",
			//	"path_info" : "/foo.*",
			//	"ip" : "127.0.0.1",
			//	"port" : 5604
			//},
		]
	}
	
}
