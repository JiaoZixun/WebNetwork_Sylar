#include"../sylar/log.h"
#include"../sylar/util.h"

int main(){
    /*
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));

    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    //新建format
    sylar::LogFormatter::ptr fmt(new sylar::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    
    //设定file_appender的日志级别，只放error以上的日志
    file_appender->setLevel(sylar::LogLevel::ERROR);
    logger->addAppender(file_appender);

    SYLAR_LOG_DEBUG(logger)<<"hello";
    SYLAR_LOG_INFO(logger)<<"hello world";
    SYLAR_LOG_ERROR(logger)<<"hello error";

    SYLAR_LOG_FMT_INFO(logger, "test fmt info is%s", "aa");
    */
    auto l = sylar::loggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(l) << "xxxxx";

    return 0;
}