#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h>
#include <cppcms/http_file.h>
#include <iostream>
#include "content.h"

using namespace std;
class uploader: public cppcms::application {
public:
    uploader(cppcms::service &s) :
        cppcms::application(s) 
    {
    }

    void main(std::string /*unused*/)
    {
        content::upload c;
        if(request().request_method()=="POST") {
            c.info.load(context());
            if(c.info.validate()) {
                // Create file name
                //
                // Note:
                // NEVER, NEVER, NEVER use user supplied file name!
                //
                // Use it to display or for general information only.
                //
                // If you would try to save the file under user supplied name you
                // may open yourself to multiple security issues like directory
                // traversal and more.
                //
                // So create your own name. If you want to keep the connection with original
                // name you may use sha1 hash and then save it.
                //
                std::string new_name = "latest_image";
                if(c.info.image.value()->mime() == "image/png")
                    new_name += ".png";
                else
                    new_name += ".jpg"; // we had validated mime-type

                //
                // Note: save_to is more efficient then reading file from
                // c.info.image.value()->data() stream and writing it
                // as save to would try to move the saved file over file-system
                // and it would be more efficient.
                //
                c.info.image.value()->save_to("./uploads/" + new_name); 
            	c.info.clear();
            }
        }
        render("upload",c);
    }
};

int main(int argc,char ** argv)
{
    try {
        cppcms::service app(argc,argv);
        app.applications_pool().mount(cppcms::applications_factory<uploader>());
        app.run();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
