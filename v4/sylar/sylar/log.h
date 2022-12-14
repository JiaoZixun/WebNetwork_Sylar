#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<vector>
#include<sstream>
#include<fstream>
#include<stdio.h>
#include<iostream>
#include<time.h>
#include<stdarg.h>
#include<map>
#include"singleton.h"
#include"thread.h"
#include"util.h"



#define SYLAR_LOG_LEVEL(logger, level)  \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(   \
                        new sylar::LogEvent(logger, __FILE__, __LINE__, 0,  \
                        sylar::GetThreadId(), sylar::GetFiberId(), time(0),  \
                        sylar::Thread::GetName(), level)  \
                    )   \
                ).getSS()


#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)


#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)    \
    if(logger->getLevel() <= level) \
        sylar::LogEventWrap(sylar::LogEvent::ptr(   \
                        new sylar::LogEvent(logger, __FILE__, __LINE__, 0,  \
                        sylar::GetThreadId(), sylar::GetFiberId(), time(0),  \
                        sylar::Thread::GetName(), level)  \
                    )   \
                ).getEvent()->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)


//?????????????????????logger???????????????
#define SYLAR_GET_ROOT() sylar::loggerMgr::GetInstance()->getRoot()

#define SYLAR_LOG_NAME(name) sylar::loggerMgr::GetInstance()->getLogger(name)

namespace sylar{

class Logger;
class LoggerManger;

//???????????????
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
    static LogLevel::Level FromString(const std::string& str);
};


//????????????
class LogEvent{
public:
    using ptr = std::shared_ptr<LogEvent>;
    //typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, const char* file, int32_t m_line, uint32_t elapse,
            uint32_t thread_id, uint32_t fiber_id, uint64_t time, 
            const std::string& thread_name, LogLevel::Level lev = LogLevel::DEBUG);


    const char* getFile() const {return m_file;}
    const int32_t getLine() const {return m_line;}
    const uint32_t getElapse() const {return m_elapse;}
    const uint32_t getThreadId() const {return m_threadId;}
    const uint32_t getFiberId() const {return m_fiberId;}
    const uint64_t getTime() const {return m_time;}
    const std::string& getThreadName() const {return m_threadName;}
    std::string getContext() const {return m_ss.str();}
    std::stringstream& getSS() {return m_ss;}
    const LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level lev)  {m_level = lev;}
    std::shared_ptr<Logger> getLogger() {return m_logger;}

    //format??????
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

private:
    std::shared_ptr<Logger> m_logger;                   //??????????????????????????????
    const char* m_file = nullptr;                       //?????????
    int32_t m_line = 0;                                 //??????
    uint32_t m_elapse = 0;                              //??????????????????????????????
    uint32_t m_threadId = 0;                            //?????????
    uint32_t m_fiberId = 0;                             //?????????
    uint64_t m_time = 0;                                //??????
    std::stringstream m_ss;                             //??????????????????
    std::string m_threadName;                           //????????????
    LogLevel::Level m_level = LogLevel::DEBUG;          //????????????
};

//?????????event??????????????????????????????????????????
class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    std::stringstream& getSS();
    LogEvent::ptr getEvent() {return m_event;}
private:
    LogEvent::ptr m_event;
};

//??????????????????
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
    void init();                                    //pattern?????????

    bool isError() const {return m_error;}

    const std::string getPattern() const {return m_pattern;}
private:
    std::string m_pattern;                          //??????pattern????????????
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;
};



//???????????????
class LogAppender{
friend class Logger;
public:
    using ptr = std::shared_ptr<LogAppender>;
    typedef Spinlock MutexType;
    //typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender(){}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;       //?????????????????????????????????
    virtual std::string toYamlString() = 0;
    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter();
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level lev) {m_level = lev;}
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    bool m_hasFormatter = false;
    // ???
    MutexType m_mutex;
    LogFormatter::ptr m_formatter;

};





//?????????
//??????enable_shared_from_this???????????????shared_from_this()??????????????????
class Logger : public std::enable_shared_from_this<Logger> {
friend class LoggerManger;
public:
    using ptr = std::shared_ptr<Logger>;
    //typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();

    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level level) {m_level = level;}

    const std::string& getName() const {return m_name;}

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter();

    std::string toYamlString();

private:
    std::string m_name;                             //????????????
    LogLevel::Level m_level;                        //?????????????????????????????????????????????
    std::list<LogAppender::ptr> m_appenders;        //Appender??????  ?????????????????????
    LogFormatter::ptr m_formatter;                  //?????????????????????????????????
    Logger::ptr m_root;
    MutexType m_mutex;
};

//logger?????????
class LoggerManger {
public:
    typedef Spinlock MutexType;
    LoggerManger();
    Logger::ptr getLogger(const std::string& name);
    void init();
    Logger::ptr getRoot() const {return m_root;}
    std::string toYamlString();
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
    MutexType m_mutex;
};




//?????????????????????Appender
class StdoutLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    //typedef std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;  //????????????
    virtual std::string toYamlString() override;
private:

};


//??????????????????Appender
class FileLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    //typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    virtual void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;  //????????????
    virtual std::string toYamlString() override;
    //?????????????????????????????????????????????true
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime = 0;
};





//????????????????????????????????????FormatItem???format??????
class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getContext();
    }
};

//??????????????????????????????LogLevel????????????????????????????????????string
class LevelFormatItem : public LogFormatter::FormatItem{
public:
    LevelFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << LogLevel::ToString(event->getLevel());
    }
};

//??????????????????
class ElapseFormatItem : public LogFormatter::FormatItem{
public:
    ElapseFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

//?????????????????????
class NameFormatItem : public LogFormatter::FormatItem{
public:
    NameFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};

//????????????id
class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
    ThreadIdFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

//??????id
class FiberIdFormatItem : public LogFormatter::FormatItem{
public:
    FiberIdFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

//????????????
class ThreadNameFormatItem : public LogFormatter::FormatItem{
public:
    ThreadNameFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

//?????????
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

//???????????????
class FilenameFormatItem : public LogFormatter::FormatItem{
public:
    FilenameFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

//?????????
class LineFormatItem : public LogFormatter::FormatItem{
public:
    LineFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

//??????
class NewLineFormatItem : public LogFormatter::FormatItem{
public:
    NewLineFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << std::endl;
    }
};

//??????????????????
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

//??????Tab
class TabFormatItem : public LogFormatter::FormatItem{
public:
    TabFormatItem(const std::string str="") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogEvent::ptr event) override {
        os << "\t";
    }
};


//????????????????????????????????????logger?????????
typedef sylar::Singleton<LoggerManger> loggerMgr;


}

#endif