#include <cppcms/json.h>
#include <iostream>
#include <sstream>

struct person {
    std::string name;
    double salary;
    std::vector<std::string> kids_names;
};

namespace cppcms {
    namespace json {

        //
        // We specilize cppcms::json::traits structure to convert
        // objects to and from json values
        //

        template<>
        struct traits<person> {
            // this function should throw
            // cppcms::json::bad_value_cast in case
            // of invalid or impossible conversion, 
            // this would give an easy way to substitute
            // a default value in case of fault
            static person get(value const &v)
            {
                person p;
                if(v.type()!=is_object)
                    throw bad_value_cast();
                
                p.name=v.get<std::string>("name");
                p.salary=v.get<double>("salary");
                p.kids_names=
                    v.get<std::vector<std::string> >("kids_names");
                // this works because generic vector and string specialized
                return p;
            }
            static void set(value &v,person const &in)
            {
                v.set("name",in.name);
                v.set("salary",in.salary);
                v.set("kids_names",in.kids_names);
            }
        };
    } // json
} // cppcms

int main()
{
    cppcms::json::value my_object;
    // Create object data
    my_object["name"]="Moshe";
    my_object["salary"]=1000.0;
    my_object["kids_names"][0]="Yossi";
    my_object["kids_names"][1]="Yonni";
    
    // this is additional data
    my_object["data"]["weight"]=85;
    my_object["data"]["height"]=1.80;

    // Get values using path.
    std::string name=my_object.get<std::string>("name");
    double salary = my_object.get<double>("salary");
    int weight = my_object.get<int>("data.weight");
    double height = my_object.get<double>("data.height");
    
    // get values using default value if not set
    std::string eyes_color = my_object.get("data.eyes_color","brown");

    // load person object from the object
    // using above traits
    person moshe = my_object.get_value<person>();

    // save person object to json object
    cppcms::json::value other = moshe;

    // write data to output
    std::cout << other << std::endl;

    // write data formatter nicely
    my_object.save(std::cout,cppcms::json::readable);

    // save object to stream and load it back
    std::stringstream tmp;
    tmp << my_object;
    cppcms::json::value reloaded;
    reloaded.load(tmp,true);
}



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
