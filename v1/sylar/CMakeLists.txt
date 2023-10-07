cmake_minimum_required(VERSION 2.8)
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")


set(LIB_SRC
    sylar/log.cpp
    sylar/util.cpp
    )

add_library(sylar SHARED ${LIB_SRC})

add_executable(test1 "tests/test.cpp" sylar "${LIBS}")
add_dependencies(test1 sylar)
target_link_libraries(test1 sylar)


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)