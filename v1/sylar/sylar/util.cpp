#include"util.h"

namespace sylar{

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}
}