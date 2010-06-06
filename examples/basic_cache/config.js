{
	"service" : {
		"api" : "http",
		"port" : 8080
	},
	"http" : {
		"script_names" : [ "/hello" ]
	},
	"cache" : {
		"backend" : "thread_shared",
		"limit" : 100,
		"memsize" : 64,
	}
}

