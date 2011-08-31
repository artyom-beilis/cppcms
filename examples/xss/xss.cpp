#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/http_request.h>
#include <cppcms/filters.h>
#include <cppcms/xss.h>
#include <cppcms/json.h>
#include <iostream>


// Loading rules is expencive but once it is loaded
// we can access same object from multiple threads
// so just create one global object
cppcms::xss::rules global_rules;

class my_xss : public cppcms::application {
public:
    my_xss(cppcms::service &srv) :
        cppcms::application(srv) 
    {
    }
    virtual void main(std::string url);
};

void my_xss::main(std::string /*url*/)
{
    std::string text;

    text = request().post("xss");


    response().out() <<  
        "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">"
        "<html>\n"
        "<head><title>XSS Test</title>\n";

    if(settings().get("filters.tinymce",false)==true) {
        response().out() <<
            "<script type=\"text/javascript\" src=\"/tinymce/jscripts/tiny_mce/tiny_mce.js\" ></script >\n"
            "<script type=\"text/javascript\" >\n"
            "tinyMCE.init({ \n"
            "  mode : \"textareas\",\n"
            "  plugins : \"autolink\",\n"
            "  theme : \"simple\" });\n"
            "</script>\n";
    }

    response().out() <<  
        "<body>\n"
        "  <h1>Test XSS Filters</h1>\n"
        "<form method='post' action='/xss'>\n"
        "<textarea name='xss' cols='80' rows='25' >" 
        << cppcms::filters::escape(text) << 
        "</textarea><br>\n"
        "<input type='submit' value='Send' />\n"
        "</form>\n"
        " <table border='1'>\n"
        "<tr><td>Original</td><td><code><pre>"
        << cppcms::filters::escape(text)
        <<"</pre></code></td></tr>\n"
        "<tr><td>Filtered</td><td><code><pre>"
        << cppcms::filters::escape(cppcms::xss::filter(text,global_rules))
        <<"</pre></code></td></tr>\n"
        "<tr><td>Filtered HTML</td><td>"
        << cppcms::xss::filter(text,global_rules)
        <<"</td></tr>\n"
        <<"</table>\n"
        "</body>\n"
        "</html>\n";
};

int main(int argc,char ** argv)
{
    try {
        cppcms::service srv(argc,argv);
        // Load rules from the profile file that the path we store in 
        // configuration file
        global_rules = cppcms::xss::rules(srv.settings().get<std::string>("filters.profile"));
        srv.applications_pool().mount(cppcms::applications_factory<my_xss>());
        srv.run();
    }
    catch(std::exception const &e) {
        std::cerr<<e.what()<<std::endl;
    }
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
