{
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script_names" : [ "/hello" ]
	},
	"localization" : {
		"encoding" : "utf-8",
		"messages" : {
			"paths" : [ "./locale" ],
			"domains" : [ "hello" ]
		},
		"locales" : [ "he_IL", "en_US" ]
	}
}
