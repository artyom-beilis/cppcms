
{
	"service" : {
		"api" : "http",
		"port" : 8080,
		"ip" : "127.0.0.1",
		"worker_threads" : 2,
		"backlog": 100,
	},
	"http" : {
		"script_names" : [ "/test" , "/async" , "/sync" ]
	}
}
