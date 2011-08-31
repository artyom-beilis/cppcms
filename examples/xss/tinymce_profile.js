{
	//
	// Profile for simple tinymce 
	// theme with autolink plugin
	//
	"encoding" : "UTF-8",
	"entities" : [ "nbsp" ],
	"tags" : {
		"opening_and_closing" : [ 
			"p",
			"ul","ol","li", // lists
			"strong", "em", 
			"span",
			"a"
		],
		"stand_alone" : [ "br" , "hr" ]
	},
	"attributes" : [
		{
			"tags" : [ "a" ],
			"attributes" : [ "href" ],
			"type" : "absolute_uri"
		},
		{
			"tags" : [ "span" ],
			"attributes" : [ "style" ],
			"type" : "regex",
			"expression" : "(\\s*text-decoration\\s*:\\s*(underline|line-through)\\s*;)*"
		}
	]
}

