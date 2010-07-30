#ifndef CONTENT_H
#define CONTENT_H

#include <cppcms/view.h>
#include <cppcms/form.h>
#include <string>



namespace content  {
 
struct input_form : public cppcms::form {
    cppcms::widgets::numeric<int> arg;
    cppcms::widgets::submit submit;

    input_form()
    {
        arg.message("N");
        submit.value("Calc");
        *this + arg + submit;
        arg.non_empty();
    }
};

struct message : public cppcms::base_content {
    long long int fact;
    int arg;
    input_form info;
};

}


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
