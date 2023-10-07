# 网络框架

## 9-15课

## 配置模块

### yaml-cpp安装路径

```
ubuntu默认安装在/usr/local/include/
```

### yaml-cpp使用

```c++
//使用LoadFile加载yaml文件，可以使用cout<<输出
YAML::Node root = YAML::LoadFile("/home/ubuntu/StudyCpp/sylar/bin/conf/log.yaml");

//yaml中node有不同的结点 分别使用不同方法遍历

// Null 空节点
// Sequence 序列，类似于一个Vector,对应YAML格式中的数组
if(node.IsSequence()){
    for(size_t i = 0; i < node.size(); ++i){
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << std::string(level * 4, ' ') 
            << i << " - " << node[i].Type() << " - " <<level;
        print_yaml(node[i], level + 1);
    }
}
// Map 类似标准库中的Map，对应YAML格式中的对象
if(node.IsMap()){
    for(auto it = node.begin();
        it != node.end(); ++it){
        SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << std::string(level * 4, ' ') 
            << it->first << " - " << it->second.Type() << " - " <<level;
        print_yaml(it->second, level + 1);
    }
}
// Scalar 标量，对应YAML格式中的常量
//Load从字符串加载转为yaml格式，在string转T的类模板中使用
YAML::Node root = YAML::Load(std::string);
```

### 调用流程

> 读入yaml，调用sylar::Config::LoadFromYaml(root);将yaml结点展平为map格式，键为字符串，值为yaml结点。找到在s_datas中存储过的进行更新，需要进行反序列化fromString，从string中恢复原有类型存入ConfigVar的m_val中。

> 静态map存储关键字对应ConfigVarBase基类指针

```c++
static ConfigVarMap s_datas;	//typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
```

> Lookup查找是否存在该关键字，若没有则存储

```c++
//用户创建
sylar::ConfigVar<float>::ptr g_float_value_config = 
    sylar::Config::Lookup("system.value", (float)10.21f, "system value");
//Lookup返回一个ConfigVar<T>类型的智能指针
//Lookup函数内部：
typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
s_datas[name] = v;		//存储的是ConfigVar对象，由父类指针指向
//在创建ConfigVar对象时，m_val存储的是真实值,例如：int a，float b，vector<T> c
template<class T, class FromStr = LexicalCast<std::string, T>
                , class ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase;	//默认tostring由T转string，fromstring由string转T
//交由LexicalCast<T, std::string>类模板实现
std::string toString() override;
//交由LexicalCast<std::string, T>类模板实现
bool fromString(const std::string& val) override;
//两个类模板都重载了()运算符，使用仿函数的方式调用
//对于STL容器需要进行偏特化
```

### 类模板

```c++
//给定T类型，从FromStr类转换为ToStr类
template<T, FromStr, ToStr>
class ConfigVar;

//转换类模板，从F类型转到T类型
template<F, T>
class LexicalCast;

//容器偏特化
//string转vector<T>
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
            vec.push_back(LexicalCast<std::string, T>() (ss.str()));	//需要取出vector中每个元素交由LexicalCast<std::string, T>实现
        }
        return vec;
    }
};
//vector<T>转string
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
```

### STL容器支持

> 对不同容器实现模板偏特化，list、set、unordered_set和vector实现相同

```c++
//string转list<T>
template<class T>
class LexicalCast<std::string, std::list<T> >;
//list<T>转string
template<class T>
class LexicalCast<std::list<T>, std::string>;
//string转set<T>
template<class T>
class LexicalCast<std::string, std::set<T> >;
//set<T>转string
template<class T>
class LexicalCast<std::set<T>, std::string>;
//string转unordered_set<T>
template<class T>
class LexicalCast<std::string, std::unordered_set<T> >;
//unordered_set<T>转string
template<class T>
class LexicalCast<std::unordered_set<T>, std::string>;
```

> map转换：map<std::string, T>

```c++
//偏特化 string --> map<std::string, T>
template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        //这里的typename是类型标识符，因为编译时还不能确定map的类型，先在此声明
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
//string转unordered_map<string, T>
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> >;
//unordered_map<string, T>转string
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string>
```



### 自定义类型支持

> 需要对自定义类型实现模板偏特化

```c++
//Person类
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
//对Person类实现模板偏特化
namespace sylar{
//string转Person
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
//Person转string
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
//使用
sylar::ConfigVar<Person>::ptr g_person_config = 
    sylar::Config::Lookup("class.person", Person(), "class person");
SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "before:" << g_person_config->getValue().toString() << " - " << g_person_config->toString();
/*
before:[Person name =  age = 0 sex = 0 ] - name: ""
age: 0
sex: false
*/
//还可以和STL容器嵌套使用
sylar::ConfigVar<std::map<std::string, Person> >::ptr g_person_map_config = 
    sylar::Config::Lookup("class.map", std::map<std::string, Person>{}, "class person map");
```



### 配置变更

> 配置发生变更后通知持有配置的所有用户 ----> 观察者模式
>
> 用户首先注册回调事件，当配置发生改变时调用回调函数，达到通知的目的

```c++
typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;	//function包装一个回调函数，需要知道原值和新值
//变更回调函数组，当注册的事件发生变化时，调用对应的回调函数
std::map<uint64_t, on_change_cb> m_cbs;
//实现对回调数组的操作
void setListener(uint64_t key, on_change_cb cb) {
    m_cbs[key] = cb;
}
void delListtener(uint64_t key) {
    m_cbs.erase(key);
}
on_change_cb getListener(uint64_t key) {
    auto it = m_cbs.find(key);
    return it == m_cbs.end() ? nullptr : it->second;
}
void clearListener() {
    m_cbs.clear();
}
//在更改配置的时候调用回调函数
void setValue(const T& v) {
    //如果改变和当前一样，直接返回
    if(v == m_val){
        return ;
    }
    //遍历回调数组 找到类型相同，但值不同的回调运行
    for(auto& i : m_cbs){
        //这里m_val是原值，v是新值
        i.second(m_val, v);
    }
    m_val = v;
}
//用户注册回调事件
g_person_config->setListener(10, [](const Person& old_val, const Person& new_val){
    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "old_val=" << old_val.toString() 
        << "new_val=" << new_val.toString();
});	//old_val=[Person name =  age = 0 sex = 0 ]new_val=[Person name = sylar age = 35 sex = 1 ]
//g_person_config是新建配置：
sylar::ConfigVar<Person>::ptr g_person_config = 
    sylar::Config::Lookup("class.person", Person(), "class person");
```







