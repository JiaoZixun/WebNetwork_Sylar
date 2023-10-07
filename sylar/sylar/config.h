#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include<memory>
#include<string>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include"log.h"
#include"yaml-cpp/yaml.h"
#include<vector>
#include<map>
#include<list>
#include<unordered_map>
#include<unordered_set>
#include<functional>
#include"thread.h"

namespace sylar{

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}
    const std::string& getName() const {return m_name;}
    const std::string& getDescription() const {return m_description;}

    //将其他类转为string类
    virtual std::string toString() = 0;
    //将string转为其他类
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;
    std::string m_description;

};

//F from_type, T to_type
template<class F, class T>
class LexicalCast {
public:
    T operator() (const F& v) {
        return boost::lexical_cast<T>(v);
    }
};

//偏特化 string --> vector<T>
template<class T>
class LexicalCast<std::string, std::vector<T> > {
public:
    std::vector<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定vector的类型，先在此声明
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};


//偏特化 vector<T> --> string
template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator() (const std::vector<T>& v) {
        YAML::Node node;
        for(auto& i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>() (i)) );
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//偏特化 string --> list<T>
template<class T>
class LexicalCast<std::string, std::list<T> > {
public:
    std::list<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定vector的类型，先在此声明
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};


//偏特化 list<T> --> string
template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator() (const std::list<T>& v) {
        YAML::Node node;
        for(auto& i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>() (i)) );
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//偏特化 string --> set<T>
template<class T>
class LexicalCast<std::string, std::set<T> > {
public:
    std::set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定vector的类型，先在此声明
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};


//偏特化 set<T> --> string
template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator() (const std::set<T>& v) {
        YAML::Node node;
        for(auto& i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>() (i)) );
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化 string --> unordered_set<T>
template<class T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定vector的类型，先在此声明
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i){
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>() (ss.str()));
        }
        return vec;
    }
};


//偏特化 unordered_set<T> --> string
template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator() (const std::unordered_set<T>& v) {
        YAML::Node node;
        for(auto& i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>() (i)) );
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化 string --> map<std::string, T>
template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定vector的类型，先在此声明
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                 it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>() (ss.str())));
        }
        return vec;
    }
};


//偏特化 map<std::string, T> --> string
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator() (const std::map<std::string, T>& v) {
        YAML::Node node;
        for(auto& it : v){
            node[it.first] = YAML::Load(LexicalCast<T, std::string>() (it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化 string --> unordered_map<std::string, T>
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定vector的类型，先在此声明
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin();
                 it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>() (ss.str())));
        }
        return vec;
    }
};


//偏特化 unordered_map<std::string, T> --> string
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator() (const std::unordered_map<std::string, T>& v) {
        YAML::Node node;
        for(auto& it : v){
            node[it.first] = YAML::Load(LexicalCast<T, std::string>() (it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//FromStr  T operator() (const std::string&)
//ToStr std::string operator() (const T&)
template<class T, class FromStr = LexicalCast<std::string, T>
                , class ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase{
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name
            , const T& default_value
            , const std::string& description = "")
        :ConfigVarBase(name, description)
        , m_val(default_value){

    }

    std::string toString() override {
        try{
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr() (m_val);
        }catch(std::exception& e){
            SYLAR_LOG_ERROR(SYLAR_GET_ROOT()) <<"ConfigVar::toString exception" 
                << e.what() << "convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }
    bool fromString(const std::string& val) override {
        try{
            //m_val = boost::lexical_cast<T>(val);
            setValue(FromStr() (val));
            return true;
        }catch(std::exception& e){
            SYLAR_LOG_ERROR(SYLAR_GET_ROOT()) <<"ConfigVar::fromString exception"
                << e.what() << "convert: string to" << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }
    void setValue(const T& v) {
        {
            RWMutexType::ReadLock lock(m_mutex);
            //如果改变和当前一样，直接返回
            if(v == m_val){
                return ;
            }
            //遍历回调数组 找到类型相同，但值不同的回调运行
            for(auto& i : m_cbs){
                i.second(m_val, v);
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v;
    }
    std::string getTypeName() const { return typeid(T).name(); }

    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_fun_id = 0;
        RWMutexType::WriteLock lock(m_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }

    void delListtener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }
    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }

private:
    T m_val;
    //变更回调函数组，当注册的事件发生变化时，调用对应的回调函数
    std::map<uint64_t, on_change_cb> m_cbs;
    RWMutexType m_mutex;
};


class Config{
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    typedef RWMutex RWMutexType;
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = "") {
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);
        //key相同但类型不同
        if(it != GetDatas().end()) {
            //如果it->second不是T类型的，dynamic_pointer_cast转换失败返回nullptr
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
            if(tmp){
                SYLAR_LOG_ERROR(SYLAR_GET_ROOT()) << "Lookup name=" << name << "exists";
                return tmp;
            }
            else{
                SYLAR_LOG_ERROR(SYLAR_GET_ROOT()) << "Lookup name =" << name << " exists but type: not " 
                    << typeid(T).name() << " real_type = " << it->second->getTypeName()              //getTypeName():基类指针不能访问派生类成员函数，必须声明虚函数
                    << " " << it->second->toString();     
                return nullptr;
            }
        }
        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._012345678")
                    != std::string::npos){
                SYLAR_LOG_ERROR(SYLAR_GET_ROOT()) << "Lookup name invaild " << name;
                throw std::invalid_argument(name);
        }
        
        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        GetDatas()[name] = v;
        return v;
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);

    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
private:
    //静态函数返回静态成员，因为静态变量的初始化顺序不定，在调用s_datas时一定保证其已被初始化
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};

}

#endif