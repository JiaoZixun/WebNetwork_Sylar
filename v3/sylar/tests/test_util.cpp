#include"sylar/sylar.h"
#include<assert.h>

sylar::Logger::ptr g_logger = SYLAR_GET_ROOT();

void test_assert() {
    SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10, 2, "      ");
    SYLAR_ASSERT2(0 == 1, "abchdj xx");
}


int main() {
    test_assert();
    return 0;
}