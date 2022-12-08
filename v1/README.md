# 网络框架

## 1-8课

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







