#include "log.h"
#include<map>
#include<functional>
#include<iostream>

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

LogEvent::LogEvent(std::shared_ptr<Logger> logger, const char* file, int32_t m_line, uint32_t elapse,
            uint32_t thread_id, uint32_t fiber_id, uint64_t time,
            LogLevel::Level lev):
    m_logger(logger),
    m_file(file),
    m_line(m_line),
    m_elapse(elapse),
    m_threadId(thread_id),
    m_fiberId(fiber_id),
    m_time(time),
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
        
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        event->setLevel(level);
        auto self = shared_from_this();
        for(auto& x:m_appenders){
            x->log(self, level, event);           //取出每个appender，父类指针调用子类函数
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
    if(!appender->getFormatter()){
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender){
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
        std::cout<<m_formatter->format(logger, event);
    }
}

// FileLogAppender类
void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event){
    if(level >= m_level){
        m_filestream << m_formatter->format(logger, event);
    }
}
FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename){
    reopen();
}

bool FileLogAppender::reopen(){
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
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
    XX(F, FiberIdFormatItem)            //协程id
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
}
Logger::ptr LoggerManger::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;
}
}