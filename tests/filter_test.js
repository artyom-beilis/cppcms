
{
	"service" : {
		"api" : "http",
		"port" : 8080,
		"ip" : "127.0.0.1",
		"backlog": 100,
		"input_buffer_size" : 8192
	},
	"http" : {
		"timeout" : 3,
		"script_names" : [ "/upload", "/raw" ]
	}
}
