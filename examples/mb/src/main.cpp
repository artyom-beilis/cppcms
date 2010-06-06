#include "mb.h"
#include <cppcms/service.h>
#include <cppcms/applications_pool.h>

using namespace std;
using namespace cppcms;

int main(int argc,char ** argv)
{
    try {
        service app(argc,argv);
        app.applications_pool().mount(applications_factory<apps::mb>());
        app.run();
    }
    catch(std::exception const &e) {
        cerr<<e.what()<<endl;
    }
}

