#include<map>
#include<functional>
#include<iostream>
#include"config.h"
#include "log.h"

namespace sylar{



//LogLevel类
const char* LogLevel::ToString(LogLevel::Level level){
    switch (level){
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
# undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string& str){
#define XX(level, v)    \
    if(str == #v) { \
        return LogLevel::level;   \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);

    return LogLevel::UNKNOW;
#undef XX
}

// 初始化列表按成员变量顺序初始
LogEvent::LogEvent(std::shared_ptr<Logger> logger, const char* file, int32_t m_line, uint32_t elapse,
            uint32_t thread_id, uint32_t fiber_id, uint64_t time,
            const std::string& thread_name, LogLevel::Level lev):
    m_logger(logger),
    m_file(file),
    m_line(m_line),
    m_elapse(elapse),
    m_threadId(thread_id),
    m_fiberId(fiber_id),
    m_time(time),
    m_threadName(thread_name),
    m_level(lev) {


}


//format方法
void LogEvent::format(const char* fmt, ...) {
    //va_list类型的指针指向各个参数
    va_list al;
    //一个宏用来确定第一个参数的位置，fmt是固定参数名称
    va_start(al, fmt);
    format(fmt, al);
    //清空可变参数列表
    va_end(al);
}
void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1){
        m_ss << std::string(buf, len);
        free(buf);
    }
}

//LogEventWrap类
LogEventWrap::LogEventWrap(LogEvent::ptr e):
        m_event(e) {

}
LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}
std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

//Logger类
Logger::Logger(const std::string& name)
    :m_name(name),
    m_level(LogLevel::DEBUG){
        
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

std::string Logger::toYamlString(){
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::UNKNOW){
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_formatter){
        node["formatter"] = m_formatter->getPattern();
    }
    
    for(auto& i : m_appenders){
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();

}

void Logger::clearAppenders() {
    MutexType::Lock lock(m_mutex);
    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr val){
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
    for(auto& x : m_appenders) {
        MutexType::Lock ll(x->m_mutex);
        if(!x->m_hasFormatter) {
            x->m_formatter = m_formatter;
        }
    }
}

void Logger::setFormatter(const std::string& val){
    sylar::LogFormatter::ptr new_val(new sylar::LogFormatter(val));
    if(new_val->isError()){
        return ;
    }
    setFormatter(new_val);
}

LogFormatter::ptr Logger::getFormatter(){
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

void LogAppender::setFormatter(LogFormatter::ptr val){
    MutexType::Lock lock(m_mutex);
    m_formatter = val;
    if(m_formatter) {
        m_hasFormatter = true;
    }
    else {
        m_hasFormatter = false;
    }
}
LogFormatter::ptr LogAppender::getFormatter() {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        event->setLevel(level);
        auto self = shared_from_this();
        MutexType::Lock lock(m_mutex);
        if(!m_appenders.empty()) {
            for(auto& x:m_appenders){
                x->log(self, level, event);           //取出每个appender，父类指针调用子类函数
            }
        }
        else if(m_root){
            m_root->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event){
    log(LogLevel::Level::DEBUG, event);
}
void Logger::info(LogEvent::ptr event){
    log(LogLevel::Level::INFO, event);
}

void Logger::warn(LogEvent::ptr event){
    log(LogLevel::Level::WARN, event);
}
void Logger::error(LogEvent::ptr event){
    log(LogLevel::Level::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event){
    log(LogLevel::Level::FATAL, event);
}
void Logger::addAppender(LogAppender::ptr appender){
    MutexType::Lock lock(m_mutex);
    if(!appender->getFormatter()){
        MutexType::Lock ll(appender->m_mutex);
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
    MutexType::Lock lock(m_mutex);
    for(auto it = this->m_appenders.begin(); it!=this->m_appenders.end(); ++it){
        if(*it == appender){
            this->m_appenders.erase(it);
            break;
        }
    }

}

// StdoutLogAppender类
void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        MutexType::Lock lock(m_mutex);
        std::cout<<m_formatter->format(logger, event);
    }
}

std::string StdoutLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if(m_level != LogLevel::UNKNOW){
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_hasFormatter && m_formatter){
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

// FileLogAppender类
void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        uint64_t now = time(0);
        //每秒重开一次文件，防止硬删除日志文件，系统无法感知
        if(now >= (m_lastTime + 3)){
            reopen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
        if(!(m_filestream << m_formatter->format(logger, event))) {
            std::cout << "error" << std::endl;
        }
    }
}

std::string FileLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::UNKNOW){
        node["level"] = LogLevel::ToString(m_level);
    }
    if(m_hasFormatter && m_formatter){
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename){
    reopen();
}

bool FileLogAppender::reopen(){
    MutexType::Lock lock(m_mutex);
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;
}

//LogFormatter类
LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern){
    init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogEvent::ptr event){
    std::stringstream ss;
    //遍历解析好的FormatItem父类对象，父类指针指向子类format实现，返回流式格式
    for(auto& x:m_items){
        x->format(ss, logger, event);
    }
    return ss.str();
}

//解析pattern
/*
%m -- 消息体
%p -- level
%r -- 启动后时间
%c -- 日志名称
%t -- 日志线程
%n -- 回车换行
%d -- 时间
%f -- 文件名
%l -- 行号
*/
void LogFormatter::init() {
    //str, format, type
	std::vector<std::tuple<std::string, std::string, int> > vec;
	auto iszimu = [](char ch){
		return ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122));
	};
	int n = m_pattern.size();
	std::string curpat;
	for(int i=0;i<n;){
		//std::cout<<"m_pattern["<<i<<"]: "<<m_pattern[i]<<std::endl;
		if(m_pattern[i]!='%'){
			curpat += m_pattern[i];
			i++;
			if(i==n){
				vec.push_back(std::make_tuple(curpat, std::string(), 0)); 
			}
			continue;
		}
		else{
			//将多余的%加入字符串中 
			while(i+1<n&&m_pattern[i+1]=='%'){
				curpat+=m_pattern[i];
				i++;
			}
			//std::cout<<"i: "<<i<<curpat<<std::endl;
			if(curpat.size()>0){
				if(i==n-1){
					curpat+=m_pattern[i];
					i++;
				}
				vec.push_back(std::make_tuple(curpat, std::string(), 0)); 
				curpat = ""; 
				continue;
			}
			if(i==n-1){
				i++;
				vec.push_back(std::make_tuple(curpat, std::string(), 1)); 
                curpat = ""; 
				continue;
			}
			if(m_pattern[i]=='%'&&i+1<n&&iszimu(m_pattern[i+1])){
				//若是字母 记录该pattern
				//判断后面是否有{}
				int patst=i+1,patend=i+1;
				while(patend<n&&iszimu(m_pattern[patend])){
					patend++;
				}
				curpat = m_pattern.substr(patst, patend-patst);
				//std::cout<<"curpat"<<curpat<<std::endl;
				
				std::string fmt;
				i = patend;
				if(i<n&&m_pattern[i]=='{'){
					int j=i+1,st=i+1;
					while(j<n&&m_pattern[j]!='}'){
						j++;
					}
					fmt = m_pattern.substr(st, j-st);
					//std::cout<<fmt<<std::endl;
					vec.push_back(std::make_tuple(curpat, fmt, 1));
					curpat = "";
					i = j+1;
					continue;
				} 
				vec.push_back(std::make_tuple(curpat, fmt, 1)); 
				curpat = "";
			}
		}
	}
    /*
	for(auto x:vec){
		std::cout<<std::get<0>(x) <<"#"<<std::get<1>(x) <<"#"<<std::get<2>(x) <<std::endl;
	}
    */
    if(!curpat.empty()){
        vec.push_back(std::make_tuple(curpat, "", 0)); 
    }

    //记录了不同关键字对应的函数指针
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C)  \
    {#str, [](const std::string& fmt) {return FormatItem::ptr (new C(fmt));}}
    XX(m, MessageFormatItem),           //错误信息
    XX(p, LevelFormatItem),             //日志级别
    XX(r, ElapseFormatItem),            //运行时间
    XX(c, NameFormatItem),              //日志器名称
    XX(t, ThreadIdFormatItem),          //线程id
    XX(n, NewLineFormatItem),           //换行
    XX(d, DateTimeFormatItem),          //时间
    XX(f, FilenameFormatItem),          //文件名
    XX(l, LineFormatItem),              //行号
    XX(T, TabFormatItem),               //Tab
    XX(F, FiberIdFormatItem),           //协程id
    XX(N, ThreadNameFormatItem)         //线程名称
#undef XX
    };

    for(auto& x:vec){
        //如果这一条pattern的类别是0，表示不用解析直接输出的字符串
        //m_items中存的是FormatItem父类指针，使用StringFormatItem子类初始化父类智能指针
        if(std::get<2>(x) == 0){
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(x) ) ));
        }
        else{
            //在s_format_items查找是否有该关键字对应的格式化类方法
            auto it = s_format_items.find(std::get<0>(x));
            if(it==s_format_items.end()){
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(x) + ">>") ));
                m_error = true;
            }
            else{
                //如果找到了，存入该关键字对应的方法类，并使用vec中第二个值初始化
                //只有日期类传入的std::get<1>(x)是一个format字符串
                m_items.push_back(it->second(std::get<1>(x)));
            }
        }
        //std::cout<<std::get<0>(x) <<"#"<<std::get<1>(x) <<"#"<<std::get<2>(x) <<std::endl;
    }
}


//LoggerManger类
LoggerManger::LoggerManger() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

    m_loggers[m_root->m_name] = m_root;

    init();
}

Logger::ptr LoggerManger::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()){
        return it->second;
    }
    //若不存在则直接创建
    Logger::ptr logger (new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

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

sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_define = 
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
    LogIniter() {
        g_log_define->addListener([](const std::set<LogDefine>& old_val,
                    const std::set<LogDefine>& new_val){
            SYLAR_LOG_INFO(SYLAR_GET_ROOT()) << "on_logger_conf_changed";
            for(auto& i : new_val){
                auto it = old_val.find(i);
                sylar::Logger::ptr logger;
                if(it == old_val.end()){
                    //新增logger
                    logger = SYLAR_LOG_NAME(i.name);
                }
                else{
                    if(!(i == *it)){
                        //修改logger
                        logger = SYLAR_LOG_NAME(i.name);
                    }
                }
                logger->setLevel(i.level);
                if(!i.formatter.empty()){
                    logger->setFormatter(i.formatter);
                }
                logger->clearAppenders();
                for(auto& a : i.appenders){
                    sylar::LogAppender::ptr ap;
                    if(a.type == 1){
                        ap.reset(new FileLogAppender(a.file));
                    }
                    else if(a.type == 2){
                        ap.reset(new StdoutLogAppender);
                    }
                    ap->setLevel(a.level);
                    if(!a.formatter.empty()) {
                        LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                        if(!fmt->isError()) {
                            ap->setFormatter(fmt);
                        }
                        else {
                            std::cout << "log.name=" << i.name << "appender type=" << a.type
                                      << "formatter=" << a.formatter <<"is invalid" << std::endl;
                        }
                    }
                    logger->addAppender(ap);
                }
            }
            for(auto& i : old_val){
                auto it = new_val.find(i);
                if(it == new_val.end()){
                    //删除logger
                    auto logger = SYLAR_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppenders();
                }
            }
        });
    }
};

//全局变量在main函数之前调用构造函数
static LogIniter __log_init;

std::string LoggerManger::toYamlString(){
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto& i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void LoggerManger::init(){

}

}