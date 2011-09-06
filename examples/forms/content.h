#ifndef CONTENT_H
#define CONTENT_H

#include <cppcms/view.h>
#include <cppcms/form.h>

#include <iostream>


namespace content  {
 
struct info_form : public cppcms::form {
    cppcms::widgets::text name;
    cppcms::widgets::radio sex;
    cppcms::widgets::select marital;
    cppcms::widgets::numeric<double> age;
    cppcms::widgets::submit submit;
    info_form()
    {
        name.message("Your Name");
        sex.message("Sex");
        marital.message("Marital Status");
        age.message("Your Age");
        submit.value("Send");
        add(name);
        add(sex);
        add(marital);
        add(age);
        add(submit);
        sex.add("Male","male");
        sex.add("Female","female");
        marital.add("Single","single");
        marital.add("Married","married");
        marital.add("Divorced","divorced");
        name.non_empty();
        age.range(0,120);
    }
    virtual bool validate()
    {
        if(!form::validate()) 
            return false;
        if(marital.selected_id()!="single" && age.value()<18) {
            marital.valid(false);
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
