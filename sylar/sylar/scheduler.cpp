#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace sylar{
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name) {
    SYLAR_ASSERT(threads > 0);
    SYLAR_LOG_INFO(g_logger) << "Scheduler() ";
    if(use_caller) {
        sylar::Fiber::GetThis();
        --threads;
        SYLAR_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        sylar::Thread::SetName(m_name);

        t_fiber = m_rootFiber.get();
        m_rootThread = sylar::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    }
    else {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    SYLAR_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return ;
    }
    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());
    m_threads.resize(m_threadCount);
    for(size_t i = 0;i<m_threadCount;++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                            , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();
}

void Scheduler::stop() {
    m_autoStop = true;
    if(m_rootFiber 
            && m_threadCount == 0
            && (m_rootFiber->getState() == Fiber:: TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        SYLAR_LOG_INFO(g_logger) << this << "stopped";
        m_stopping = true;

        if(stopping()) {
            return ;
        }
    }

    //bool exit_on_this_fiber = false;
    if(m_rootThread != -1) {
        SYLAR_ASSERT(GetThis() == this);
    }
    else {
        SYLAR_ASSERT(GetThis() != this);
    }
    m_stopping = true;
    for(size_t i=0;i<m_threadCount;++i) {
        tickle();
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        if(!stopping()) {
            m_rootFiber->call();
        }
    }
    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }
    for(auto& x:thrs) {
        x->join();
    }

}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    set_hook_enable(true);
    setThis();
    if(sylar::GetThreadId() != m_rootThread) {
        t_fiber = Fiber::GetThis().get();
    }
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    SYLAR_LOG_INFO(g_logger) << "run";
    while(true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if(it->thread != -1 && it->thread != sylar::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }
                SYLAR_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }
                ft = *it;
                m_fibers.erase(it);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
        }

        if(tickle_me) {
            tickle();
        }
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();
            --m_activeThreadCount;
            if(ft.fiber->getState() == Fiber::READY) {
                scheduler(ft.fiber);
            }
            else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        }
        else if(ft.cb) {
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            }
            else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                scheduler(cb_fiber);
                cb_fiber.reset();
            }
            else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            }
            else{
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        }
        else {
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}


void Scheduler::tickle() {
    SYLAR_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    SYLAR_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        sylar::Fiber::YieldToHold();
    }
}


}