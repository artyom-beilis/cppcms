
{
	"service" : {
		"api" : "http"
	},
	"http" : {
		"script" : "/test"
	},
	"session" : {
		"timeout" : 5, 
		"client_size_limit" : 256,
		"cookies" : {
			"prefix" : "sc",
			"domain" : "foo.bar",
			"path" : "/foo",
		},
		"client" : {
			"hmac" : "sha1",
			"hmac_key" : "dc07e0ff8e44be872e86fe848841584cd38983e5" 
		},
		"server" : {
			"storage" : "memory",
		}
	},
}
