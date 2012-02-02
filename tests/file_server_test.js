
{
	"service" : {
		"worker_threads" : 5,
		"api" : "http"

	},
	"http": {
		"rewrite" : [{ "regex" : "^/rewrite_me/(.*)$", "pattern" : "/$1" }]
	},
	"file_server" : {
		"enable" : true,
		"document_root" : "file_server/www",
		"alias" : [
			{
				"url" : "/alias",
				"path" : "file_server/al"
			}
		]
	}
}
