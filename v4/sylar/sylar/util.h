#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__


#include<pthread.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<stdio.h>
#include<unistd.h>
#include<stdint.h>
#include<vector>
#include<string>


namespace sylar{

pid_t GetThreadId();
uint64_t GetFiberId();

//输出函数调用栈信息
void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);
std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");

}


#endif