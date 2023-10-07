# 网络框架

## 三、16-25课

## 日志模块+配置模块整合

```c++
/*
0.创建log文件对应的配置指针	log.cpp:590
sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_define = 
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

在构造函数中，加入该配置对应的回调函数，并将其声明为静态变量，使其在main之前初始化
struct LogIniter{
	LogIniter() {
		g_log_define->addListener(匿名函数)
	}
}
static LogIniter __log_init;

1.加载yaml文件	sylar::Config::LoadFromYaml(root);
2.LoadFromYaml中解析yaml，将yaml格式化为string：yaml；依次取出，判断key是否存在于std::map<std::string, ConfigVarBase::ptr>中
3.通过ConfigVarBase基类指针调用ConfigVar::fromString()方法，执行setValue()方法
4.setValue中若旧值和新值不同，触发回调函数
*/
```

### 1) LogDefine

> log的基本配置信息

```c++
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
```

### 2) LogAppenderDefine

> logappender的基本配置信息

```c++
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
```

### 3) LogDefine和set之间的偏特化

```c++
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
```

### 4)  toYamlString()

> 实现将当前配置输出为string
>
> LoggerManger::toYamlString()  ---》  Logger::toYamlString() ---》 LogAppender::toYamlString() 虚函数  -->  StdoutLogAppender::toYamlString()
>
> ​																																							|
>
> ​																																						   V
>
> ​																																	FileLogAppender::toYamlString()

## 

## 线程模块

> 通过封装semaphore和pthread实现

### 1) 信号量

```c++
class Semaphore {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void notify();
private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:
    sem_t m_semaphore;
}; 
```

### 2) 锁模板

>通过模板的方式实现，离开锁的作用域后先解锁，再析构锁

```c++
template<class T>
struct ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
            m_mutex.lock();
            m_locked = true;
    }
    ~ScopedLockImpl() {
        unlock();
    }
    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }
    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};
```

### 3) 互斥锁

> 同一时间只允许一个线程使用共享资源，读写速度较慢
>
> 包含ScopedLockImpl< Mutex >，当锁离开了作用域后先调用ScopedLockImpl::~ScopedLockImpl()析构函数实现解锁操作，再进行析构锁

```c++
class Mutex{
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }
    void lock() {
        pthread_mutex_lock(&m_mutex);
    }
    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};
```

### 4) 读写锁

> 读写分离，封装pthread_rwlock
>
> 速度较快，适合读多写少的情况

```c++
//读写模板
template<class T>
struct ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
            m_mutex.rdlock();
            m_locked = true;
    }
    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};
template<class T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
            m_mutex.wrlock();
            m_locked = true;
    }
    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};
//读写锁
class RWMutex {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }
    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }
    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};
```

### 5) 自旋锁

>适用于快任务，每次线程操作时间较短
>
>若无法抢占任务时，锁进行自旋操作，不会立即挂起，而是一直等待上一个线程，可以设置等待时间
>
>会发生死锁，如果等待的线程一直阻塞，自旋锁会一直自旋等待；在自旋锁还未释放时再次申请了一个自旋锁

```c++
class Spinlock {
public:
    typedef ScopedLockImpl<Spinlock> Lock;
    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }
    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }
    void lock() {
        pthread_spin_lock(&m_mutex);
    }
    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};
```

### 6) CAS乐观锁

> 无锁编程，容易引发临界资源的问题
>
> 封装原子操作，核心是比较并交换，如果想要插入的位置不是最后则一直向后移动，直至末尾开始插入

```c++
/*
A：当前值
V：内存值
B：新值
CAS(A, V, B){
	if(A == V){
		V = B;
	}
	else {
		重试或放弃
	}
}
ABA问题：A从5改为10 成功，B从5改为10 阻塞，C从10改为5 成功；这时候B采用CAS(5,5,10)判断成功，将5又改为了10，出现问题
解决：CAS判断时加入版本号，进行版本比较
*/
class CASLock {
public:
    typedef ScopedLockImpl<CASLock> Lock;
    CASLock() {
        m_mutex.clear();
    }
    ~CASLock() {
    }
    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }
    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    volatile std::atomic_flag m_mutex;
};
```

### 7) 线程

```c++
class Thread{
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();
    pid_t getId() const {return m_id;}
    const std::string& getName() const {return m_name;}
    void join();
    static Thread* GetThis();
    static const std::string& GetName();
    static void SetName(const std::string& name);
private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    static void* run(void* arg);
private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;
};
```

## 日志+配置+线程整合

### 1) 日志

> 使用自旋锁，在LogAppender、Logger、LoggerManger类中加入
>
> 多线程情况时，访问资源的函数都需要考虑加锁

```c++
std::string Logger::toYamlString();
void Logger::clearAppenders();
void Logger::setFormatter(LogFormatter::ptr val);
LogFormatter::ptr Logger::getFormatter();
void LogAppender::setFormatter(LogFormatter::ptr val);
LogFormatter::ptr LogAppender::getFormatter();
void Logger::log(LogLevel::Level level, LogEvent::ptr event);
void Logger::addAppender(LogAppender::ptr appender);
void Logger::delAppender(LogAppender::ptr appender);
void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
std::string StdoutLogAppender::toYamlString();
void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
std::string FileLogAppender::toYamlString();
bool FileLogAppender::reopen();
Logger::ptr LoggerManger::getLogger(const std::string& name);
std::string LoggerManger::toYamlString();
```

### 2) 配置

> 使用读写锁，读多写少的情况

```c++
//ConfigVar类：
std::string toString();
const T getValue();
void setValue(const T& v);
uint64_t addListener(on_change_cb cb);
void delListtener(uint64_t key);
on_change_cb getListener(uint64_t key);
void clearListener();
//Config类
void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb)
template<class T>
static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = "");
template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name);
//ConfigVarBase类
ConfigVarBase::ptr Config::LookupBase(const std::string& name);
```









































