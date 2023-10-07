#include"sylar/sylar.h"
#include<unistd.h>

sylar::Logger::ptr g_logger1 = SYLAR_GET_ROOT();

int count = 0;
//sylar::RWMutex s_mutex;
sylar::Mutex s_mutex;

void func1() {
    SYLAR_LOG_INFO(g_logger1) << " name:" << sylar::Thread::GetName() 
                              << " this.name: " <<sylar::Thread::GetThis()->getName()
                              << " this.id: " << sylar::Thread::GetThis()->getId();
    
    for(int i=0;i<10;i++){
        //sylar::RWMutex::WriteLock lock(s_mutex);
        sylar::Mutex::Lock lock(s_mutex);
        count++;
    }
}

void func2() {
    while(true) {
        SYLAR_LOG_INFO(g_logger1) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
    
}
void func3() {
    while(true) {
        SYLAR_LOG_INFO(g_logger1) << "================================================================";
    }
}

int main(){
    SYLAR_LOG_INFO(g_logger1) << "thread test brgin";
    YAML::Node root = YAML::LoadFile("/home/jzx/StudyCpp/sylar/bin/conf/log2.yaml");
    sylar::Config::LoadFromYaml(root);
    std::vector<sylar::Thread::ptr> thrs;
    for(int i=0;i<2;i++){
        sylar::Thread::ptr thr(new sylar::Thread(&func2, "name_"+ std::to_string(i * 2)));
        //sylar::Thread::ptr thr2(new sylar::Thread(&func3, "name_"+ std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        //thrs.push_back(thr2);
    }
    
    for(size_t i=0;i<thrs.size();i++){
        thrs[i]->join();
    }
    SYLAR_LOG_INFO(g_logger1) << "thread test end";
    SYLAR_LOG_INFO(g_logger1) << "count = " << count;

    
    return 0;
}