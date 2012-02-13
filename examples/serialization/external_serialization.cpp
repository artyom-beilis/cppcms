#include <cppcms/serialization.h>
#include <cppcms/application.h>
#include <cppcms/cache_interface.h>
#include <cppcms/session_interface.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>
#include <iostream>

struct person {

    std::string name;
    std::string family;

    template<typename Archive>
    void serialize(Archive &a,unsigned /*version*/)
    {
        a & name & family;
    }
};

template<typename T>
void load_from_boost(std::string const &src,T &obj)
{
    std::stringbuf ss(src);
    {
        boost::archive::binary_iarchive oa(ss);
        oa >> obj;
    }
}


template<typename T>
void save_with_boost(T const &obj,std::string &tgt)
{
    std::stringbuf ss;
    {
        boost::archive::binary_oarchive oa(ss);
        oa << obj;
    }
    tgt=ss.str();
}

void init_person(person &john)
{
    john.name = "John";
    john.family = "Jonson";
}

/* Specialize */
namespace cppcms {
    template<>
    struct serialization_traits<person> {
        static void save(person const &obj,std::string &tgt)
        {
            save_with_boost(obj,tgt);
        }
        static void load(std::string const &src,person &obj)
        {
            load_from_boost(src,obj);
        }
    };
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
            std::cout << john.name << std::endl;
        }

    }
    return 0;
}





// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
