cmake_minimum_required(VERSION 2.8)
project(sylar)

include(cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})

set(LIB_SRC
    sylar/config.cpp
    sylar/fiber.cpp
    sylar/log.cpp
    sylar/thread.cpp
    sylar/util.cpp
    )

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)  # 重定向__FILE__

# add_executable(test1 "tests/test.cpp" sylar "${LIBS}")
# add_dependencies(test1 sylar)
# target_link_libraries(test1 sylar)


set(
    LIB_LIB
    sylar
    pthread
    ${YAMLCPP}
)


add_executable(test1 tests/test.cpp sylar "${LIBS}")
add_dependencies(test1 sylar)
force_redefine_file_macro_for_sources(test)  # 重定向__FILE__
target_link_libraries(test1 sylar ${LIB_LIB})

add_executable(test_config tests/test_config.cpp sylar "${LIBS}")
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config)  # 重定向__FILE__
target_link_libraries(test_config sylar ${LIB_LIB})

add_executable(test_thread tests/test_thread.cpp sylar "${LIBS}")
add_dependencies(test_thread sylar)
force_redefine_file_macro_for_sources(test_thread)  # 重定向__FILE__
target_link_libraries(test_thread sylar ${LIB_LIB})

add_executable(test_util tests/test_util.cpp sylar "${LIBS}")
add_dependencies(test_util sylar)
force_redefine_file_macro_for_sources(test_util)  # 重定向__FILE__
target_link_libraries(test_util sylar ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cpp sylar "${LIBS}")
add_dependencies(test_fiber sylar)
force_redefine_file_macro_for_sources(test_fiber)  # 重定向__FILE__
target_link_libraries(test_fiber sylar ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)