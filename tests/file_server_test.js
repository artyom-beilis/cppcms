
{
	"service" : {
		"worker_threads" : 5,
		"api" : "http"

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
