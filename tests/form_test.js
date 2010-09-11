
{
	"service" : {
		"api" : "http",
		"port" : 8080,
		"ip" : "127.0.0.1",
	},
	"http" : {
		"script" : "/test"
	},
	"localization" : {
		"messages" : {
			"paths" : [ "./tests/locale" ],
			"domains" : [ "test" ]
		},
		"locales" : [ "C.UTF-8" ],
	},
	
}
