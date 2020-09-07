
{
	"service" : {
		"worker_threads" : 5
	},
	"http" : {
		"script_names" : [ "/test" , "/async" , "/sync", "/nonblocking" ],
        "timeout" : 5
	},
	"localization" : {
		"messages" : {
			"paths" : [ "./tests/locale" ],
			"domains" : [ "test" ]
		},
		"locales" : [ "he_IL.UTF-8" ],
	}
}
