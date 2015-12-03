
{
	"service" : {
		"api" : "http",
		"port" : 8080,
		"ip" : "127.0.0.1",
		"worker_threads" : 3 // 1 for ::system("python ...") and another 2 for applications, so effectively only 2 are active
	},
	"http" : {
		"script_names" : [ "/test" , "/async" , "/sync" ]
	}
}
