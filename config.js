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
	"l10n" : {
		"defaults" : {
			"encoding" : "UTF-8",
			"locale" : "en_US",
			"std_locale" : [ "ICU" , "C" ]
		},
		"messages" : {
			"paths" : [ "./transtext/locale" ],
			"domains" : [ "app", "test" ],
			"default_domain" : ["test"]
		},
		"locales" : [
			"he_IL",
			"he_IL@calendar=hebrew",
			"en_US",
			{ "locale" : "ru_RU" , "std_locale" : "C" }
		]
	},
	"locale" : {
		"locales" : 
			[ 
				"he_IL.UTF-8",
				"en_US.UTF-8",
				"he_IL.cp1255",
				"tr_TR.UTF-8" ],	
		//"default" : "tr_TR.UTF-8",
				// list of supported languages
		//"default" :  "he_IL.cp1255",			// default language (default first one)
		"default" :  "he_IL.UTF-8",			// default language (default first one)
		"gettext_domains" : [ "app", "test" ],		// list of supported domains
		"default_gettext_domain" :  "test",		// default domain (default first one)
		"gettext_path" : "./transtext/locale",		// path to locale directory
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

	"file_server" : {
		"enable" : true,
		"doument_root" : "."
		// "mime_types" : "mime.type"
	}
	
}
