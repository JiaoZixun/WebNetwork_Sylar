#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();

void test_fiber() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << " test in fiber s_count=" << s_count;
    sleep(1);
    if(--s_count >= 0) {
        sylar::Scheduler::GetThis()->scheduler(&test_fiber, sylar::GetThreadId());
    }
    
}

int main() {
    SYLAR_LOG_INFO(g_logger) << " main ";
    sylar::Scheduler sc(3, false, "test");
    
    sc.start();
    sleep(2);
    sc.scheduler(&test_fiber);
    SYLAR_LOG_INFO(g_logger) << " scheduler ";
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << " over ";
    return 0;
}