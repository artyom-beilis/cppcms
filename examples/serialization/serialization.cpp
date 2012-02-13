#include <cppcms/serialization.h>
#include <cppcms/application.h>
#include <cppcms/cache_interface.h>
#include <cppcms/session_interface.h>
#include <iostream>


// POD structure
struct physical {
    int age;
    double height;
};


struct child : public cppcms::serializable {
    std::string name;
    physical parameters;

	void serialize(cppcms::archive &a)
    {
        a & name & cppcms::as_pod(parameters);
    }

};

struct person : public cppcms::serializable {

    std::string name;
    std::string family;
    std::string occupation;
    std::string martial_state;
    physical parameters;
    std::list<child> children;
    std::vector<std::string> friends;

    void serialize(cppcms::archive &a)
    {
        a & name & family & occupation & martial_state & cppcms::as_pod(parameters) & children & friends;
    }
};

void init_person(person &john)
{
    john.name = "John";
    john.family = "Jonson";
    john.occupation = "Programmer";
    john.martial_state = "married";
    john.parameters.age = 30;
    john.parameters.height = 1.80;
    john.children.push_back(child());
    john.children.back().name = "Tom";
    john.children.back().parameters.age = 5;
    john.children.back().parameters.height = 1.0;
    john.friends.push_back("Gorge");
}

class app : public cppcms::application {
public:
    app(cppcms::service &s) : cppcms::application(s)
    {
    }

    void main(std::string /*url*/)
    {
        {
            person john;
            session().fetch_data("me",john);
            john.parameters.age += 1;
            session().store_data("me",john);
        }
        {
            person john;
            if(!cache().fetch_data("john",john)) {
                init_person(john);
                cache().store_data("john",john);
            }
        }
    }
};

int main()
{
    { // Using archive directly
        std::string buffer; 
        {
            person john;
            init_person(john);
            cppcms::archive a;
            john.save(a);
            buffer = a.str();
        }
        {
            person john;
            cppcms::archive a;
            a.str(buffer);
            john.load(a);
            std::cout << john.name << " " << john.children.front().name << " " << john.friends.front() << std::endl;
        }
    }
    { // Generic way
        std::string buffer;
        {
            person john;
            init_person(john);
            cppcms::serialization_traits<person>::save(john,buffer);
        }
        {
            person john;
            cppcms::serialization_traits<person>::load(buffer,john);
            std::cout << john.name << " " << john.children.front().name << " " << john.friends.front() << std::endl;
        }

    }
    return 0;
}





// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
