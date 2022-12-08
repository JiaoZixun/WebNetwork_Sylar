#include "../sylar/log.h"
#include "../sylar/config.h"
#include <yaml-cpp/yaml.h>


sylar::ConfigVar<int>::ptr g_int_value_config = 
    sylar::Config::Lookup("system.port", (int)8080, "system port");

//同一名字，类型不可更改 Lookup中error
sylar::ConfigVar<float>::ptr g_floatx_value_config = 
    sylar::Config::Lookup("system.port", (float)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config = 
    sylar::Config::Lookup("system.value", (float)10.21f, "system value");

sylar::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config = 
    sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2, 3}, "system int vec");

sylar::ConfigVar<std::list<int> >::ptr g_int_list_value_config = 
    sylar::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");

sylar::ConfigVar<std::set<int> >::ptr g_int_set_value_config = 
    sylar::Config::Lookup("system.int_set", std::set<int>{11, 22}, "system int set");

sylar::ConfigVar<std::unordered_set<int> >::ptr g_int_unset_value_config = 
    sylar::Config::Lookup("system.int_unset", std::unordered_set<int>{2, 1}, "system int unset");

sylar::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config = 
    sylar::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k", 21}}, "system str_int map");

sylar::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_unmap_value_config = 
    sylar::Config::Lookup("system.str_int_unmap", std::unordered_map<std::string, int>{{"k", 21}, {"k2", 10}}, "system str_int unmap");

void print_yaml(const YAML::Node& node, int level){
    if(node.IsScalar()){
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << std::string(level * 4, ' ') 
            << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if(node.IsNull()){
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << std::string(level * 4, ' ') 
            << "NULL - " << node.Type() <<level;
    }
    else if(node.IsMap()){
        for(auto it = node.begin();
                it != node.end(); ++it){
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << std::string(level * 4, ' ') 
                << it->first << " - " << it->second.Type() << " - " <<level;
            print_yaml(it->second, level + 1);
        }
    }
    else if(node.IsSequence()){
        for(size_t i = 0; i < node.size(); ++i){
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << std::string(level * 4, ' ') 
                << i << " - " << node[i].Type() << " - " <<level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml(){
    YAML::Node root = YAML::LoadFile("/home/ubuntu/StudyCpp/sylar/bin/conf/test.yaml");

    print_yaml(root, 0);

    //SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << root;
}

void test_config() {
    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "before:" << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "before:" << g_float_value_config->getValue();
#define XX(g_var, name, prefix) \
    {   \
        auto& v = g_var->getValue();    \
        for(auto& i : v){   \
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << #prefix " " #name " : " << i;    \
        }   \
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << #prefix " " #name " yaml: " << g_var->toString();    \
    }

#define XX_M(g_var, name, prefix) \
    {   \
        auto& v = g_var->getValue();    \
        for(auto& it : v){   \
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << #prefix " " #name " : (" << it.first << " - " << it.second << ")";    \
        }   \
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << #prefix " " #name " yaml: " << g_var->toString();    \
    }
    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_unset_value_config, int_unset, before);

    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_unmap_value_config, str_int_unmap, before);


    YAML::Node root = YAML::LoadFile("/home/ubuntu/StudyCpp/sylar/bin/conf/test.yaml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "after:" << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "after:" << g_float_value_config->getValue();

    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_unset_value_config, int_unset, after);

    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_unmap_value_config, str_int_unmap, after);

}



class Person{
public:
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss  << "[Person name = " <<m_name
            <<" age = " << m_age
            <<" sex = " << m_sex
            <<" ]";
        return ss.str();
    }

    bool operator== (const Person& other) const {
        return this->m_name == other.m_name
                &&this->m_age == other.m_age
                &&this->m_sex == other.m_sex;
    }
};

sylar::ConfigVar<Person>::ptr g_person_config = 
    sylar::Config::Lookup("class.person", Person(), "class person");

sylar::ConfigVar<std::map<std::string, Person> >::ptr g_person_map_config = 
    sylar::Config::Lookup("class.map", std::map<std::string, Person>{}, "class person map");

sylar::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_vec_map_config = 
    sylar::Config::Lookup("class.vecmap", std::map<std::string, std::vector<Person> >{}, "class person vec map");

namespace sylar{
template<>
class LexicalCast<std::string, Person> {
public:
    Person operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定vector的类型，先在此声明
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator() (const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
}


void test_class(){
    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "before:" << g_person_config->getValue().toString() << " - " << g_person_config->toString();
#define XX_PM(g_var, prefix)    \
    {   \
        auto x = g_var->getValue();   \
        for(auto& i : x){   \
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << prefix << " : " << i.first << " - " << i.second.toString(); \
        }   \
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << prefix << " : size() = " << x.size();  \
    }
    XX_PM(g_person_map_config, "class.map before");

#define XXPVM(g_var, prefix)    \
    {   \
        auto x = g_var->getValue();   \
        for(auto& i:x){ \
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << prefix << " : " << i.first << " = [ ";   \
            for(auto& j:i.second){  \
                SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << j.toString() << ", ";   \
            }   \
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << " ] "; \
        }   \
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << prefix << " : size = " << x.size();     \
    }

    XXPVM(g_person_vec_map_config, "class.vecmap before");

    //注册更改的回调函数
    g_person_config->addListener([](const Person& old_val, const Person& new_val){
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "old_val = " << old_val.toString() 
                << " new_val = " << new_val.toString();
    });

    YAML::Node root = YAML::LoadFile("/home/ubuntu/StudyCpp/sylar/bin/conf/test.yaml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "after:" << g_person_config->getValue().toString() << " - " << g_person_config->toString();
    XX_PM(g_person_map_config, "class.map after");
    XXPVM(g_person_vec_map_config, "class.vecmap after");
}
/*
*/
void test_log() {
    static sylar::Logger::ptr system_log = SYLAR_LOG_NAME("root");
    std::cout << sylar::loggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/jzx/StudyCpp/sylar/bin/conf/log.yaml");
    sylar::Config::LoadFromYaml(root);
    std::cout << "=======================================" << std::endl;
    std::cout << sylar::loggerMgr::GetInstance()->toYamlString() << std::endl;

    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "hello world" << std::endl;
    system_log->setFormatter("%d - %m%n");
    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "hello world" << std::endl;

}
int main(){
    //test_config();
    //test_class();
    test_log();
    sylar::Config::Visit([](sylar::ConfigVarBase::ptr var) {
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "name = " << var->getName()
                                  << "description = " << var->getDescription()
                                  << "type = " << var->getTypeName() 
                                  << "value = " << var->toString();
    });
    return 0;
}