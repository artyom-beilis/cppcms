{
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script" : "/hello"
	},
	"localization" : {
		"messages" : {
			"paths" : [ "./locale" ],
			"domains" : [ "hello" ]
		},
		"locales" : [ "he_IL.UTF-8", "en_US.UTF-8" ]
	}
}
