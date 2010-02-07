
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
		"encoding" : "UTF-8",
		"messages" : {
			"paths" : [ "./tests/locale" ],
			"domains" : [ "test" ]
		},
		"locales" : [ "he_IL" ],
	},
	
}
