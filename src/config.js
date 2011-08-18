/// CppCMS Configuration file
// Extended JSON format, "//" like comments are allowed

{
	"id" : 1,
	// Service description
	"service" : {
		//"worker_processes" : 5,
		"worker_threads" : 5,
		//"worker_threads" : 25,
	//	"api" : "scgi",
		"api" : "http",
		"port" : 8080,
	//	"port" : 8081,
		"ip" : "0.0.0.0",
	//	"ip" : "127.0.0.1",
	//	 "socket" : "/tmp/scgi.socket",
	//	"disable_global_exit_handling" , false
	//	"reactor" : "default" , // "select", "poll", "epoll", "devpoll", "kqueue"
	//	"generate_http_headers" : false,
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
		],
		//"output_buffer_size" : 16384
	},
	"daemon" : {
		//"enable" : true,
		"lock" : "lock.pid",
		"user" : "artik",
		"group" : "nobody",
		"chroot" : "/tmp",
		"fdlimit" : 16384
	},
	"http" : {
		"proxy" : {
			//"behind" : true,
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
		//"backend" : "winapi",
		"messages" : {
			"paths" : [ "./transtext/locale" ],
			"domains" : [ "test", "app" ]
		},
		"locales" : [ "en_US.UTF-8", "en_US.UTF-8", "he_IL.UTF-8@calendar=hebrew" ],
		"disable_charset_in_content_type" : false	// Disable 
	},
	"session" : {
		"expire" : "browser",
		"timeout" : 600, // seconds
		"cookies" : {
			"prefix" : "cppcms_session",
			"domain" : "",
			"path" : "/",
			"secure" : false
		},
		"location" : "client",
	//	"location" : "server",
	//	"location" : "both",
		"client_size_limit" : 1000,
		"gc" : 10,
		"client" : { 
			//"encryptor" : "hmac-sha1",  // "hmac" = "hmac-sha1", "hmac-md5", "hmac-sha224", "hmac-sha384", "hmac-sha256", "hmac-sha512"
			//"encryptor" : "hmac-sha512",  // "hmac" = "hmac-sha1", "hmac-md5", "hmac-sha224", "hmac-sha384", "hmac-sha256", "hmac-sha512"
			"encryptor" : "aes", 
			// aes or hmac -- hmac -- signature only, aes -- encryption and signature
			//"key_file" : "key.txt", //261965ba80a79c034c9ae366a19a2627261965ba80a79c034c9ae366a19a2626"
			"key" : "261965ba80a79c034c9ae366a19a2627261965ba80a79c034c9ae366a19a2626"
			//"key_file" : "261965ba80a79c034c9ae366a19a2627261965ba80a79c034c9ae366a19a2626"
			//"cbc" : "aes",
			//"hmac" : "sha1",
			//"hmac_key_file": "key.txt",
			//"cbc_key_file": "key.txt",
			//"cbc_key_file" : "261965ba80a79c034c9ae366a19a2627",
			//"hmac_key_file" : "261965ba80a79c034c9ee366a19a2628"
			//"cbc_key" : "261965ba80a79c034c9ae366a19a2627",
			//"hmac_key" : "261965ba80a79c034c9ee366a19a2628"
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
		"document_root" : "."
		// "mime_types" : "mime.type"
	},
	"logging" : {
		//"level" : "info", // "debug",
		//"level" : "debug",
		"stderr" : true,
		"file" : {
			"name" : "cppcms.log" ,
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
	},
	"security" : {
		// "multipart_form_data_limit" : 65536, // KB
		// "content_length_limit" : 1024, // KB
		// "uploads_path" : "" // temporary directory
		"display_error_message" : true,
		"csrf" : {
			"enable" : true, 	// enable CSRF prevention support - default off
			//"automatic" : false,	// check of all widgets automatically or manually - defaut - true=automatic
			"exposed" : true,	// expose CSRF token in the cookie
		}
	}
	
}
