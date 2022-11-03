#ifndef __SYLAR_SINGLETON_H__
#define __SYLAR_SINGLETON_H__

#include<memory>

namespace sylar{
template<typename T, typename X = void, int N = 0>
class Singleton{
public:
    static T* GetInstance() {
        static T v;
        //printf("T:xxxx");
        return &v;
    }
};

template<typename T, typename X = void, int N = 0>
class SingletonPtr{
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        //printf("Tptr:xxxx");
        return v;
    }
};
}

#endif