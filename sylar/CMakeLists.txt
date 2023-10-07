cmake_minimum_required(VERSION 3.0)
project(sylar)

include(cmake/utils.cmake)

# set(CMAKE_VERBOSE_MAKEFILE ON)
# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
# set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


include_directories(.)
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
# link_directories(/usr/local/lib64)

# option(BUILD_TEST "ON for complile test" OFF)

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})

set(LIB_SRC
    sylar/address.cpp
    sylar/bytearray.cpp
    sylar/config.cpp
    sylar/fd_manager.cpp
    sylar/fiber.cpp
    sylar/http/http.cpp
    sylar/http/httpclient_parser.cpp
    sylar/http/http_connection.cpp
    sylar/http/http_parser.cpp
    sylar/http/http11_parser.cpp
    sylar/http/http_session.cpp
    sylar/http/http_server.cpp
    sylar/http/servlet.cpp
    sylar/hook.cpp
    sylar/log.cpp
    sylar/scheduler.cpp
    sylar/socket.cpp
    sylar/socket_stream.cpp
    sylar/stream.cpp
    sylar/tcp_server.cpp
    sylar/timer.cpp
    sylar/thread.cpp
    sylar/util.cpp
    sylar/uri.cpp
    sylar/iomanager.cpp
    sylar/service/service_base.cpp
    sylar/service/service_check.cpp
    sylar/service/service_demo.cpp
    )


add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)  # 重定向__FILE__

# add_executable(test1 "tests/test.cpp" sylar "${LIBS}")
# add_dependencies(test1 sylar)
# target_link_libraries(test1 sylar)


set(
    LIB_LIB
    sylar
    dl
    pthread
    ${YAMLCPP}
    jsoncpp
)



add_executable(test1 tests/test.cpp sylar "${LIBS}")
add_dependencies(test1 sylar)
force_redefine_file_macro_for_sources(test1) # 重定向__FILE__
target_link_libraries(test1 sylar ${LIB_LIB})

add_executable(test_config tests/test_config.cpp sylar "${LIBS}")
add_dependencies(test_config sylar)
force_redefine_file_macro_for_sources(test_config) # 重定向__FILE__
target_link_libraries(test_config sylar ${LIB_LIB})

add_executable(test_thread tests/test_thread.cpp sylar "${LIBS}")
add_dependencies(test_thread sylar)
force_redefine_file_macro_for_sources(test_thread) # 重定向__FILE__
target_link_libraries(test_thread sylar ${LIB_LIB})

add_executable(test_util tests/test_util.cpp sylar "${LIBS}")
add_dependencies(test_util sylar)
force_redefine_file_macro_for_sources(test_util) # 重定向__FILE__
target_link_libraries(test_util sylar ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cpp sylar "${LIBS}")
add_dependencies(test_fiber sylar)
force_redefine_file_macro_for_sources(test_fiber) # 重定向__FILE__
target_link_libraries(test_fiber sylar ${LIB_LIB})

add_executable(test_scheduler tests/test_scheduler.cpp sylar "${LIBS}")
add_dependencies(test_scheduler sylar)
force_redefine_file_macro_for_sources(test_scheduler) # 重定向__FILE__
target_link_libraries(test_scheduler sylar ${LIB_LIB})

add_executable(test_iomanager tests/test_iomanager.cpp sylar "${LIBS}")
add_dependencies(test_iomanager sylar)
force_redefine_file_macro_for_sources(test_iomanager) # 重定向__FILE__
target_link_libraries(test_iomanager sylar ${LIB_LIB})

add_executable(test_hook tests/test_hook.cpp sylar "${LIBS}")
add_dependencies(test_hook sylar)
force_redefine_file_macro_for_sources(test_hook) # 重定向__FILE__
target_link_libraries(test_hook sylar ${LIB_LIB})

add_executable(test_address tests/test_address.cpp sylar "${LIBS}")
add_dependencies(test_address sylar)
force_redefine_file_macro_for_sources(test_address) # 重定向__FILE__
target_link_libraries(test_address sylar ${LIB_LIB})

add_executable(test_socket tests/test_socket.cpp sylar "${LIBS}")
add_dependencies(test_socket sylar)
force_redefine_file_macro_for_sources(test_socket) # 重定向__FILE__
target_link_libraries(test_socket sylar ${LIB_LIB})

add_executable(test_bytearray tests/test_bytearray.cpp sylar "${LIBS}")
add_dependencies(test_bytearray sylar)
force_redefine_file_macro_for_sources(test_bytearray) # 重定向__FILE__
target_link_libraries(test_bytearray sylar ${LIB_LIB})

add_executable(test_http tests/test_http.cpp sylar "${LIBS}")
add_dependencies(test_http sylar)
force_redefine_file_macro_for_sources(test_http) # 重定向__FILE__
target_link_libraries(test_http sylar ${LIB_LIB})

add_executable(test_http_parser tests/test_http_parser.cpp sylar "${LIBS}")
add_dependencies(test_http_parser sylar)
force_redefine_file_macro_for_sources(test_http_parser) # 重定向__FILE__
target_link_libraries(test_http_parser sylar ${LIB_LIB})

add_executable(test_tcp_server tests/test_tcp_server.cpp sylar "${LIBS}")
add_dependencies(test_tcp_server sylar)
force_redefine_file_macro_for_sources(test_tcp_server) # 重定向__FILE__
target_link_libraries(test_tcp_server sylar ${LIB_LIB})

add_executable(echo_server examples/echo_server.cpp sylar "${LIBS}")
add_dependencies(echo_server sylar)
force_redefine_file_macro_for_sources(echo_server) # 重定向__FILE__
target_link_libraries(echo_server sylar ${LIB_LIB})

add_executable(test_http_server tests/test_http_server.cpp sylar "${LIBS}")
add_dependencies(test_http_server sylar)
force_redefine_file_macro_for_sources(test_http_server) # 重定向__FILE__
target_link_libraries(test_http_server sylar ${LIB_LIB})

add_executable(test_http_connection tests/test_http_connection.cpp sylar "${LIBS}")
add_dependencies(test_http_connection sylar)
force_redefine_file_macro_for_sources(test_http_connection) # 重定向__FILE__
target_link_libraries(test_http_connection sylar ${LIB_LIB})

add_executable(test_uri tests/test_uri.cpp sylar "${LIBS}")
add_dependencies(test_uri sylar)
force_redefine_file_macro_for_sources(test_uri) # 重定向__FILE__
target_link_libraries(test_uri sylar ${LIB_LIB})

add_executable(sylar_demo examples/sylar_demo.cpp sylar "${LIBS}")
add_dependencies(sylar_demo sylar)
force_redefine_file_macro_for_sources(sylar_demo) # 重定向__FILE__
target_link_libraries(sylar_demo sylar ${LIB_LIB})


# sylar_add_executable(test_uri "tests/test_uri.cpp" sylar "${LIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)