// CppCMS Configuration file
// JSON format

{
	// Service description
	"service" : {
		"procs" : 0,
		"worker_threads": 5,
		"api" : "http",
		"port" : 8080,
		"ip" : "127.0.0.1"
		//  "socket" : "/tmp/scgi.socket"
	},
	"http" : {
		"script_names" : [ "/stock", "/hello" ]
	},
	"gzip" : {
		"enable" : true, // Default true
		// "level" : 1,
		// "buffer" : 4096
	},
	"locale" : {
		"locales" : [ "he_IL.UTF-8", "en_US.UTF-8" ],	// list of supported languages
		"default" :  "he_IL.UTF-8",			// default language (default first one)
		"gettext_domains" : [ "app", "test" ],		// list of supported domains
		"default_gettext_domain" :  "test",		// default domain (default first one)
		"gettext_path" : "./transtext/locale"		// path to locale directory
	},
	"session" : {
		"expire" : "browser",
		"timeout" : 10,
		"cookies_prefix" : "cppcms_session",
		"cookies_domain" : "",
		"cookies_path" : "/",
		"cookies_secure" : false
	},

	"file_server" : {
		"enable" : true,
		"doument_root" : ".",
		"mime_types" : "mime.type"
	}
	
}
