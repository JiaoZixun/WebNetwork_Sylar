# 网络框架

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
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";				// 2
    sylar::Fiber::YieldToHold();									// 交换控制权，返回主协程
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";					// 4
    sylar::Fiber::YieldToHold();									// 交换控制权，返回主协程
}																    // 实际上fiber中的MainFunc运行结束，触发销毁，析构子协程 id：1
void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "main begin -1";				    // 0
    {
        sylar::Fiber::GetThis();									// 初始化主协程，调用私有构造
        SYLAR_LOG_INFO(g_logger) << "main begin";					// 1
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));	// 创建子协程 id：1
        fiber->swapIn();											// 交换控制权，进入子协程
        SYLAR_LOG_INFO(g_logger) << "main after swapIn";			// 3
        fiber->swapIn();											// 交换控制权，进入子协程
        SYLAR_LOG_INFO(g_logger) << "main after end";				// 5
        fiber->swapIn();											// 交换控制权，进入子协程
    }
    SYLAR_LOG_INFO(g_logger) << "main after end2";					// 6
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





