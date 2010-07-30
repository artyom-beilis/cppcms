#ifndef CONTENT_H
#define CONTENT_H

#include <cppcms/view.h>
#include <cppcms/form.h>

#include <iostream>


namespace content  {
 
struct info_form : public cppcms::form {
    cppcms::widgets::text name;
    cppcms::widgets::radio sex;
    cppcms::widgets::select martial;
    cppcms::widgets::numeric<double> age;
    cppcms::widgets::submit submit;
    info_form()
    {
        name.message("Your Name");
        sex.message("Sex");
        martial.message("Martial State");
        age.message("Your Age");
        submit.value("Send");
        *this + name + sex + martial + age + submit;
        sex.add("Male","male");
        sex.add("Female","female");
        martial.add("Single","single");
        martial.add("Married","married");
        martial.add("Divorced","divorced");
        name.non_empty();
        age.range(0,120);
    }
    virtual bool validate()
    {
        if(!form::validate()) 
            return false;
        if(martial.selected_id()!="single" && age.value()<18) {
            martial.valid(false);
            return false;
        }
        return true;
    }
};

struct message : public cppcms::base_content {
    std::string name,state,sex;
    double age;
    info_form info;
};

} // content


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
