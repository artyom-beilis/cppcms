
{
	"service" : {
		"api" : "http"
	},
	"http" : {
		"script" : "/test"
	},
	"session" : {
		"timeout" : 100, 
		"location" : "server",
		"server" : {
			"storage" : "memory",
		}
	},
	"security" : {
		"multipart_form_data_limit" : 2, // KB
		"content_length_limit" : 1, // KB
		"csrf" : { "enable" : true }
	},
}
