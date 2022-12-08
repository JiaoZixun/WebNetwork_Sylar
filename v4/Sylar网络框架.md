# 网络框架

## 一、1-8课

## 日志模块

### 1）Logger

> 面向用户最上层的类：提供log接口供用户使用
>
> 可以设置输出地appender、日志级别、以及默认解析格式

```c++
//日志器
//继承enable_shared_from_this才可以使用shared_from_this()获取自身指针
class Logger : public std::enable_shared_from_this<Logger> {
public:
    using ptr = std::shared_ptr<Logger>;
    //typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level level) {m_level = level;}
    const std::string& getName() const {return m_name;}
private:
    std::string m_name;                             //日志名称
    LogLevel::Level m_level;                        //只有满足这个日志级别的才被输出
    std::list<LogAppender::ptr> m_appenders;        //Appender集合  存储了父类指针
    LogFormatter::ptr m_formatter;                  //设置一个默认的解析格式
};
```

### 2）LogAppender

> 日志输出的基类，其派生类实现log函数
>
> Logger类的log函数使用父类指针调用其派生类的log函数完成输出
>
> 可以设置LogFormatter作为解析规则

```c++
//日志输出地
class LogAppender{
public:
    using ptr = std::shared_ptr<LogAppender>;
    //typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){}
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;       //纯虚函数，交由子类实现
    void setFormatter(LogFormatter::ptr val) {m_formatter = val;}
    LogFormatter::ptr getFormatter() const {return m_formatter;}
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level lev) {m_level = lev;}
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
};

```

#### 1.StdoutLogAppender

> 输出到控制台的Appender
>
> 实现基类log方法

```c++
//输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    //typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;  //在此实现
private:
};
```

#### 2.FileLogAppender

> 输出到文件的Appender
>
> 实现基类log方法

```c++
//输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    //typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;  //在此实现
    //重新打开文件，文件打开成功返回true
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
};
```

### 3）LogFormatter

> 解析格式化输出规则的类
>
> 初始化时接受一个解析规则，也可以用默认的（在logger类构造函数设置默认解析规则）
>
> 内嵌FormatItem类，解析规则的基类，不同命令继承自该类，并实现format函数，传出ostream流对象
>
> 在构造函数里调用init函数，按顺序将不同命令对应的派生类存储到m_items中，然后format调用FormatItem::format函数得到流对象

```c++
//日志格式化器
class LogFormatter{
public:
    using ptr = std::shared_ptr<LogFormatter>;
    //typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger> logger, LogEvent::ptr event);
public:
    class FormatItem{
    public:
        using ptr = std::shared_ptr<FormatItem>;
        //typedef std::shared_ptr<FormatItem> ptr;
        //FormatItem(const std::string& fmt = "") {}
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) = 0;
    };
    void init();                                    //pattern的解析
private:
    std::string m_pattern;                          //根据pattern解析信息
    std::vector<FormatItem::ptr> m_items;

};
```

#### 其余命令派生类

```c++
//获取错误信息类：实现父类FormatItem的format方法
class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getContext();
    }
};

//获取日志级别类：调用LogLevel类的静态方法将枚举转换为string
class LevelFormatItem : public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << LogLevel::ToString(event->getLevel());
    }
};

//获取开始时间
class ElapseFormatItem : public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

//获取日志器名称
class NameFormatItem : public LogFormatter::FormatItem{
public:
    NameFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

//获取线程id
class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

//协程id
class FiberIdFormatItem : public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

//日期类
class DateTimeFormatItem : public LogFormatter::FormatItem{
public:
    DateTimeFormatItem(const std::string format = "%Y-%m-%d %H:%M:%S")
        :m_format(format){
        if(m_format.empty()){
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

//日志名称类
class FilenameFormatItem : public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

//行号类
class LineFormatItem : public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

//换行
class NewLineFormatItem : public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << std::endl;
    }
};

//输出字符串类
class StringFormatItem : public LogFormatter::FormatItem{
public:
    StringFormatItem(const std::string& str)
        :m_string(str){}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

//输出Tab
class TabFormatItem : public LogFormatter::FormatItem{
public:
    TabFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << "\t";
    }
};
```

### 4）LogEvent

> 事件类：保存事件的各项信息

```c++
//日志事件
class LogEvent{
public:
    using ptr = std::shared_ptr<LogEvent>;
    //typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, const char* file, int32_t m_line, uint32_t elapse,
            uint32_t thread_id, uint32_t fiber_id, uint64_t time, 
            LogLevel::Level lev = LogLevel::DEBUG);
    const char* getFile() const {return m_file;}
    const int32_t getLine() const {return m_line;}
    const uint32_t getElapse() const {return m_elapse;}
    const uint32_t getThreadId() const {return m_threadId;}
    const uint32_t getFiberId() const {return m_fiberId;}
    const uint64_t getTime() const {return m_time;}
    std::string getContext() const {return m_ss.str();}
    std::stringstream& getSS() {return m_ss;}
    const LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level lev)  {m_level = lev;}
    std::shared_ptr<Logger> getLogger() {return m_logger;}
    //format方法
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);
private:
    std::shared_ptr<Logger> m_logger;                   //该事件属于哪个日志器
    const char* m_file = nullptr;                       //文件名
    int32_t m_line = 0;                                 //行号
    uint32_t m_elapse = 0;                              //程序启动到开始的时间
    uint32_t m_threadId = 0;                            //线程号
    uint32_t m_fiberId = 0;                             //协程号
    uint64_t m_time = 0;                                //时间
    std::stringstream m_ss;                             //字符串输出流
    LogLevel::Level m_level = LogLevel::DEBUG;          //日志级别
};
```

### 5）日志级别类

> 表示日志级别，枚举类

```c++
//日志级别类
class LogLevel{
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const char* ToString(LogLevel::Level level);
};
```

### 6）宏

#### 1.流式

> 搭配LogEventWrap类使用，该类中包含LogEvent类智能指针作为成员，在析构时调用LogEvent类的log函数完成日志写入
>
> LogEvent类可能被多次引用，析构函数只在引用计数为零时调用，现在需要引用计数减一时就写入，因此将事件包装在LogEventWrap类中

```c++
//为了在event析构时，触发日志写入，搭配宏
class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    std::stringstream& getSS();
    LogEvent::ptr getEvent() {return m_event;}
private:
    LogEvent::ptr m_event;
};
```



```c++
#define SYLAR_LOG_LEVEL(logger, level)  \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(   \
                        new sylar::LogEvent(logger, __FILE__, __LINE__, 0,  \
                        sylar::GetThreadId(), sylar::GetFiberId(), time(0), level)  \
                    )   \
                ).getSS()


#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)
```



#### 2. 格式化输出

> 可变参数：[C++可变参数详解 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/369181555)
>
> __ VA_ARGS __ 表示可变参数列表

```c++
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)    \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(   \
                        new sylar::LogEvent(logger, __FILE__, __LINE__, 0,  \
                        sylar::GetThreadId(), sylar::GetFiberId(), time(0), level)  \
                    )   \
                ).getEvent()->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

```



### 7）其他

#### 1.LoggerManger

> 创建一个logger管理类，包含一个名字对应logger智能指针的map集合，以及一个初始logger指针

```c++
//logger管理器
class LoggerManger {
public:
    LoggerManger();
    Logger::ptr getLogger(const std::string& name);

    void init();
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};
//使用单例模板声明一个单例的管理器
typedef sylar::Singleton<LoggerManger> loggerMgr;	//懒汉模式
```



#### 2.单例模板

```c++
#ifndef __SYLAR_SINGLETON_H__
#define __SYLAR_SINGLETON_H__
#include<memory>
namespace sylar{
template<typename T, typename X = void, int N = 0>
class Singleton{
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
};
template<typename T, typename X = void, int N = 0>
class SingletonPtr{
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};
}
#endif
```



### 8）调用流程

```c++
#include"../sylar/log.h"
#include"../sylar/util.h"

int main(){
    ......
    //从logger管理器中查找一个名为xx的logger，如果没有就使用默认logger
    auto l = sylar::loggerMgr::GetInstance()->getLogger("xx");
    //调用宏完成日志输出
    SYLAR_LOG_INFO(l) << "xxxxx";
	//结果：
    //2022-10-27 10:08:26     21020   0       [INFO]  [root]  /home/ubuntu/StudyCpp/sylar/tests/test.cpp:25   xxxxx
    return 0;
}
SYLAR_LOG_INFO(l) << "xxxxx";    --->   #define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_LEVEL(logger, level)  \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(   \
                        new sylar::LogEvent(logger, __FILE__, __LINE__, 0,  \
                        sylar::GetThreadId(), sylar::GetFiberId(), time(0), level)  \
                    )   \
                ).getSS()
其中LogEventWrap析构时调用    --->    m_event->getLogger()->log(m_event->getLevel(), m_event);
在Logger类中的log为：
void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        event->setLevel(level);
        auto self = shared_from_this();
        for(auto& x:m_appenders){
            x->log(self, level, event);           //取出每个appender，父类指针调用子类函数
        }
    }
}
x->log(self, level, event); 调用 Appender子类中的log方法    --->    virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override; 实现父类log虚函数
子类Appender的log方法调用Formatter的format方法
void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        std::cout<<m_formatter->format(logger, event);
    }
}
Formatter类中std::string format(std::shared_ptr<Logger> logger, LogEvent::ptr event);方法遍历存储FormatItem的子类解析不同命令，存入流对象

virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) = 0;//交由子类实现
```



> logger->log          --->         appender->log          --->         fotmatter->format          --->         formatitem->format
>
> ​     void                <---                 输出                  <---                   string                   <---                  ostring



## 二、9-15课

## 配置模块

### 1) yaml-cpp安装路径

```
ubuntu默认安装在/usr/local/include/
```

### 2) yaml-cpp使用

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

### 3) 调用流程

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

### 4) 类模板

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

### 5) STL容器支持

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



### 6) 自定义类型支持

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



### 7) 配置变更

> 配置发生变更后通知持有配置的所有用户 ----> 观察者模式
>
> 用户首先注册回调事件，当配置发生改变时调用回调函数，达到通知的目的

```c++
typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;	//function包装一个回调函数，需要知道原值和新值
//变更回调函数组，当注册的事件发生变化时，调用对应的回调函数
std::map<uint64_t, on_change_cb> m_cbs;
//实现对回调数组的操作
void addListener(uint64_t key, on_change_cb cb) {
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
g_person_config->addListener(10, [](const Person& old_val, const Person& new_val){
    SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "old_val=" << old_val.toString() 
        << "new_val=" << new_val.toString();
});	//old_val=[Person name =  age = 0 sex = 0 ]    new_val=[Person name = sylar age = 35 sex = 1 ]
//g_person_config是新建配置：
sylar::ConfigVar<Person>::ptr g_person_config = 
    sylar::Config::Lookup("class.person", Person(), "class person");
```

## 三、16-25课

## 日志模块+配置模块整合

```c++
/*
0.创建log文件对应的配置指针	log.cpp:590
sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_define = 
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

在构造函数中，加入该配置对应的回调函数，并将其声明为静态变量，使其在main之前初始化
struct LogIniter{
	LogIniter() {
		g_log_define->addListener(匿名函数)
	}
}
static LogIniter __log_init;

1.加载yaml文件	sylar::Config::LoadFromYaml(root);
2.LoadFromYaml中解析yaml，将yaml格式化为string：yaml；依次取出，判断key是否存在于std::map<std::string, ConfigVarBase::ptr>中
3.通过ConfigVarBase基类指针调用ConfigVar::fromString()方法，执行setValue()方法
4.setValue中若旧值和新值不同，触发回调函数
*/
```

### 1) LogDefine

> log的基本配置信息

```c++
struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& oth) const {
        return name == oth.name
            && level == oth.level
            && formatter == oth.formatter
            && appenders ==oth.appenders;
    }

    bool operator<(const LogDefine& oth) const {
        return name < oth.name;
    }
};
```

### 2) LogAppenderDefine

> logappender的基本配置信息

```c++
struct LogAppenderDefine {
    int type = 0;       //1 File  2 Stdout
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type
            && level == oth.level
            && formatter == oth.formatter
            && file == oth.file;
    }
};
```

### 3) LogDefine和set之间的偏特化

```c++
template<>
class LexicalCast<std::string, std::set<LogDefine> > {
public:
    std::set<LogDefine> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        std::set<LogDefine> vec;
        for(size_t i = 0; i < node.size(); ++i){
            auto n=node[i];
            if(!n["name"].IsDefined()) {
                std::cout<<"log config error: name is null, " << n
                         << std::endl;
                continue;
            }
            LogDefine ld;
            ld.name = n["name"].as<std::string>();
            ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
            if(n["formatter"].IsDefined()) {
                ld.formatter = n["formatter"].as<std::string>();
            }
            if(n["appenders"].IsDefined()){
                for(size_t x = 0; x < n["appenders"].size(); ++x) {
                    auto a = n["appenders"][x];
                    if(!a["type"].IsDefined()) {
                        std::cout << "log config error: appenders.type is null, " << a << std::endl;
                        continue;
                    }
                    std::string type = a["type"].as<std::string>();
                    LogAppenderDefine lad;

                    if(type == "FileLogAppender"){
                        lad.type = 1;
                        if(!a["file"].IsDefined()){
                            std::cout << "log config error: fileappenders.file is null, " << a << std::endl;
                            continue;
                        }
                        lad.file = a["file"].as<std::string>();
                    }
                    else if(type == "StdoutLogAppender"){
                        lad.type = 2;
                    }
                    else{
                        std::cout << "log config error: appenders.type is invaild, " << a << std::endl;
                        continue;
                    }
                    ld.appenders.push_back(lad);
                }
            }
            vec.insert(ld);
        }
        return vec;
    }
};

template<>
class LexicalCast<std::set<LogDefine>, std::string> {
public:
    std::string operator() (const std::set<LogDefine>& v) {
        YAML::Node node;
        for(auto& i : v){
            YAML::Node n;
            n["name"] = i.name;
            if(i.level != LogLevel::UNKNOW) {
                n["level"] = LogLevel::ToString(i.level);
            }
            
            if(i.formatter.empty()){
                n["formatter"] = i.formatter;
            }
            for(auto& a:i.appenders){
                YAML::Node na;
                if(a.type==1){
                    na["type"] = "FileLogAppender";
                    na["file"] = a.file;
                }
                else if(a.type==2){
                    na["type"] = "StdoutLogAppender";
                }
                if(a.level != LogLevel::UNKNOW) {
                    na["level"] = LogLevel::ToString(a.level);
                }
                if(!a.formatter.empty()){
                    na["formatter"] = a.formatter;
                }
                n["appenders"].push_back(na);
            }
            node.push_back(n);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
```

### 4)  toYamlString()

> 实现将当前配置输出为string
>
> LoggerManger::toYamlString()  ---》  Logger::toYamlString() ---》 LogAppender::toYamlString() 虚函数  -->  StdoutLogAppender::toYamlString()
>
> ​																																							|
>
> ​																																						   V
>
> ​																																	FileLogAppender::toYamlString()

## 

## 线程模块

> 通过封装semaphore和pthread实现

### 1) 信号量

```c++
class Semaphore {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void notify();
private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:
    sem_t m_semaphore;
}; 
```

### 2) 锁模板

>通过模板的方式实现，离开锁的作用域后先解锁，再析构锁

```c++
template<class T>
struct ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
            m_mutex.lock();
            m_locked = true;
    }
    ~ScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};
```

### 3) 互斥锁

> 同一时间只允许一个线程使用共享资源，读写速度较慢
>
> 包含ScopedLockImpl< Mutex >，当锁离开了作用域后先调用ScopedLockImpl::~ScopedLockImpl()析构函数实现解锁操作，再进行析构锁

```c++
class Mutex{
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock() {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};
```

### 4) 读写锁

> 读写分离，封装pthread_rwlock
>
> 速度较快，适合读多写少的情况

```c++
//读写模板
template<class T>
struct ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
            m_mutex.rdlock();
            m_locked = true;
    }
    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};
template<class T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
            m_mutex.wrlock();
            m_locked = true;
    }
    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};
//读写锁
class RWMutex {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }
    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }
    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};
```

### 5) 自旋锁

>适用于快任务，每次线程操作时间较短
>
>若无法抢占任务时，锁进行自旋操作，不会立即挂起，而是一直等待上一个线程，可以设置等待时间
>
>会发生死锁，如果等待的线程一直阻塞，自旋锁会一直自旋等待；在自旋锁还未释放时再次申请了一个自旋锁

```c++
class Spinlock {
public:
    typedef ScopedLockImpl<Spinlock> Lock;
    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }
    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }
    void lock() {
        pthread_spin_lock(&m_mutex);
    }
    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};
```

### 6) CAS乐观锁

> 无锁编程，容易引发临界资源的问题
>
> 封装原子操作，核心是比较并交换，如果想要插入的位置不是最后则一直向后移动，直至末尾开始插入

```c++
/*
A：当前值
V：内存值
B：新值
CAS(A, V, B){
	if(A == V){
		V = B;
	}
	else {
		重试或放弃
	}
}
ABA问题：A从5改为10 成功，B从5改为10 阻塞，C从10改为5 成功；这时候B采用CAS(5,5,10)判断成功，将5又改为了10，出现问题
解决：CAS判断时加入版本号，进行版本比较
*/
class CASLock {
public:
    typedef ScopedLockImpl<CASLock> Lock;
    CASLock() {
        m_mutex.clear();
    }
    ~CASLock() {
    }
    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }
    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    volatile std::atomic_flag m_mutex;
};
```

### 7) 线程

```c++
class Thread{
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();
    pid_t getId() const {return m_id;}
    const std::string& getName() const {return m_name;}
    void join();
    static Thread* GetThis();
    static const std::string& GetName();
    static void SetName(const std::string& name);
private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    static void* run(void* arg);
private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;
};
```

## 日志+配置+线程整合

### 1) 日志

> 使用自旋锁，在LogAppender、Logger、LoggerManger类中加入
>
> 多线程情况时，访问资源的函数都需要考虑加锁

```c++
std::string Logger::toYamlString();
void Logger::clearAppenders();
void Logger::setFormatter(LogFormatter::ptr val);
LogFormatter::ptr Logger::getFormatter();
void LogAppender::setFormatter(LogFormatter::ptr val);
LogFormatter::ptr LogAppender::getFormatter();
void Logger::log(LogLevel::Level level, LogEvent::ptr event);
void Logger::addAppender(LogAppender::ptr appender);
void Logger::delAppender(LogAppender::ptr appender);
void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
std::string StdoutLogAppender::toYamlString();
void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
std::string FileLogAppender::toYamlString();
bool FileLogAppender::reopen();
Logger::ptr LoggerManger::getLogger(const std::string& name);
std::string LoggerManger::toYamlString();
```

### 2) 配置

> 使用读写锁，读多写少的情况

```c++
//ConfigVar类：
std::string toString();
const T getValue();
void setValue(const T& v);
uint64_t addListener(on_change_cb cb);
void delListtener(uint64_t key);
on_change_cb getListener(uint64_t key);
void clearListener();
//Config类
void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb)
template<class T>
static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = "");
template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name);
//ConfigVarBase类
ConfigVarBase::ptr Config::LookupBase(const std::string& name);
```





## 四、26-29课

## 协程模块

### 1) 封装assert

> 封装了assert.h断言，实现断言后能够打印出函数栈的信息，使用了execinfo.h中backtrace()和backtrace_symbols()两个函数
>
> backtrace()：传入申请的存储堆栈信息的内存指针，获取堆栈的大小，返回一共有几层堆栈
>
> backtrace_symbols()：传入存储堆栈信息的指针，堆栈层数，返回堆栈信息的字符串
>
> 按层数输出字符串即可

```c++
//实现两个宏函数，判别x，并输出信息
#define SYLAR_ASSERT(x) \
    if(!(x)) { \
        SYLAR_LOG_ERROR(SYLAR_GET_ROOT()) << "ASSERTION: " << #x \
            << "\nbacktrace:\n"  \
            <<sylar::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define SYLAR_ASSERT2(x, w)  \
    if(!(x)) { \
        SYLAR_LOG_ERROR(SYLAR_GET_ROOT()) << "ASSERTION: " << #x \
            << "\n" << w \
            << "\nbacktrace:\n"  \
            <<sylar::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }
```



### 2)  协程

> 默认在线程中起的第一个协程为主协程，子协程每次调用完毕后都回到主协程，由主协程发起每次调用
>
> 类中包含：ucontext_t结构体，其中uc_link表示上下文下一个调度的协程，uc_stack.ss_sp表示协程栈的内存地址，uc_stack.ss_size表示协程栈的大小
>
> 包含协程状态，INIT初始，HOLD挂起，EXEC执行，TERM结束，READY准备，EXCEPT出错，以及协程对应的执行函数

```c++
#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__
#include<memory>
#include<ucontext.h>
#include<functional>
#include"thread.h"
namespace sylar {
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCEPT
    };
private:
    Fiber();
public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();
    //重置协程函数，并重置状态
    //INIT TERM
    void reset(std::function<void()> cb);
    //切换到当前协程执行
    void swapIn();
    //切换到后台执行
    void swapOut();
    uint64_t getId() const { return m_id;}
public:
    //设置当前协程
    static void SetThis(Fiber* f);
    //返回当前协程
    static Fiber::ptr GetThis();
    //协程切换到后台，并且设置为Ready状态
    static void YieldToReady();
    //协程切换到后台，并且设置为Hold状态
    static void YieldToHold();
    //总协程数
    static uint64_t TotalFibers();
    static void MainFunc();
    static uint64_t GetFiberId();
private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;
    ucontext_t m_ctx;
    void* m_stack = nullptr;
    std::function<void()> m_cb;
};
}
#endif
```



### 3) 使用

```c++
// 需要先GetThis初始化一个main协程,调用私有构造函数，t_threadfiber指向主协程
sylar::Fiber::GetThis();
// 生成协程，绑定执行函数，调用有参构造函数
sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));
//t_fiber指向当前协程，执行swapcontext()，当前协程开始执行
fiber->swapIn();
//在静态函数YieldToHold()中，通过静态GetThis()函数获得当前运行协程的智能指针，将控制权交出
sylar::Fiber::YieldToHold();
//实际上每个协程与MainFunc()绑定，在其中运行传入的cb()，每次交换控制权会再次回到MainFunc()->cb()中，若MainFunc()->cb()运行完毕，执行MainFunc()的销毁，将当前协程的智能指针计数-1，然后swapout返回主协程
```

```c++
#include"sylar/sylar.h"
sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();
void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";					// 2
    sylar::Fiber::YieldToHold();									  // 交换控制权，返回主协程
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";					    // 4
    sylar::Fiber::YieldToHold();									  // 交换控制权，返回主协程
}																   // 实际上fiber中的MainFunc运行结束，触发销毁，析构子协程 id：1
void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "main begin -1";						// 0
    {
        sylar::Fiber::GetThis();									   // 初始化主协程，调用私有构造
        SYLAR_LOG_INFO(g_logger) << "main begin";						// 1
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));		  // 创建子协程 id：1
        fiber->swapIn();											  // 交换控制权，进入子协程
        SYLAR_LOG_INFO(g_logger) << "main after swapIn";				 // 3
        fiber->swapIn();											  // 交换控制权，进入子协程
        SYLAR_LOG_INFO(g_logger) << "main after end";					// 5
        fiber->swapIn();											  // 交换控制权，进入子协程
    }
    SYLAR_LOG_INFO(g_logger) << "main after end2";						 // 6
}																	// 析构主协程
int main() {
    sylar::Thread::SetName("main");
    test_fiber();
    return 0;
}
/*
2022-12-08 12:29:42     68861   main    0       [INFO]  [root]  tests/test_fiber.cpp:15 main begin -1
2022-12-08 12:29:42     68861   main    0       [DEBUG] [system]        sylar/fiber.cpp:50      Fiber::Fiber
2022-12-08 12:29:42     68861   main    0       [INFO]  [root]  tests/test_fiber.cpp:18 main begin
2022-12-08 12:29:42     68861   main    0       [DEBUG] [system]        sylar/fiber.cpp:67      Fiber::Fiber id: 1
2022-12-08 12:29:42     68861   main    1       [INFO]  [root]  tests/test_fiber.cpp:8  run_in_fiber begin
2022-12-08 12:29:42     68861   main    0       [INFO]  [root]  tests/test_fiber.cpp:22 main after swapIn
2022-12-08 12:29:42     68861   main    1       [INFO]  [root]  tests/test_fiber.cpp:10 run_in_fiber end
2022-12-08 12:29:42     68861   main    0       [INFO]  [root]  tests/test_fiber.cpp:24 main after end
2022-12-08 12:29:42     68861   main    0       [DEBUG] [system]        sylar/fiber.cpp:87      Fiber::~Fiber id: 1
2022-12-08 12:29:42     68861   main    0       [INFO]  [root]  tests/test_fiber.cpp:27 main after end2
2022-12-08 12:29:42     68861   main    0       [DEBUG] [system]        sylar/fiber.cpp:87      Fiber::~Fiber id: 0
*/
```



































