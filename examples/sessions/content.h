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
        sex.add("Male","m");
        sex.add("Female","f");
        martial.add("Single","s");
        martial.add("Married","m");
        martial.add("Divorced","d");
        name.non_empty();
        age.range(0,120);
    }
    virtual bool validate()
    {
        if(!form::validate()) 
            return false;
        if(martial.selected_id()!="s" && age.value()<18) {
            martial.valid(false);
            return false;
        }
        return true;
    }
};

struct message : public cppcms::base_content {
    std::string name,who;
    double age;
    info_form info;
};

} // content


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
