cmake_minimum_required(VERSION 3.16)
project(mrpc)

# 设置c++标准
set(CMAKE_CXX_STANDARD 20)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -g -Wall -O0 -Wno-unused-variable -pthread -ldl")

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) # 设置可执行文件输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)    # 设置库文件输出路径


# 设置头文件路径
include_directories(
    # ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/
    "/usr/local/include/google"   # protobuf头文件路径
    "/usr/local/include/tinyxml"  # tinyxml头文件路径
)

set(CMAKE_LIBRARY_PATH "/usr/local/lib") # 设置库文件路径
set(LIBS
	${CMAKE_LIBRARY_PATH}/libprotobuf.a
	${CMAKE_LIBRARY_PATH}/libtinyxml2.a
)


# common目录源文件
set(SOURCE_FILE_COMMON
    ${PROJECT_SOURCE_DIR}/src/common/util.cc
    ${PROJECT_SOURCE_DIR}/src/common/msg_id_util.cc
    ${PROJECT_SOURCE_DIR}/src/common/log.cc
    ${PROJECT_SOURCE_DIR}/src/common/log_stream.cc
	${PROJECT_SOURCE_DIR}/src/common/config.cc
	${PROJECT_SOURCE_DIR}/src/common/mutex.h
)

# net目录源文件
set(SOURCE_FILE_NET
	${PROJECT_SOURCE_DIR}/src/net/eventloop.cc
	${PROJECT_SOURCE_DIR}/src/net/fd_event.cc
	${PROJECT_SOURCE_DIR}/src/net/fd_event_group.cc
	${PROJECT_SOURCE_DIR}/src/net/wakeup_fd_event.cc
	${PROJECT_SOURCE_DIR}/src/net/timer.cc
	${PROJECT_SOURCE_DIR}/src/net/timer_event.cc
	${PROJECT_SOURCE_DIR}/src/net/io_thread.cc
	${PROJECT_SOURCE_DIR}/src/net/io_thread_pool.cc

	${PROJECT_SOURCE_DIR}/src/net/tcp/tcp_buffer.cc
	${PROJECT_SOURCE_DIR}/src/net/tcp/tcp_acceptor.cc
	${PROJECT_SOURCE_DIR}/src/net/tcp/tcp_server.cc
	${PROJECT_SOURCE_DIR}/src/net/tcp/tcp_client.cc
	${PROJECT_SOURCE_DIR}/src/net/tcp/tcp_connection.cc
	${PROJECT_SOURCE_DIR}/src/net/tcp/net_addr.cc

	${PROJECT_SOURCE_DIR}/src/net/coder/abstract_protocol.cc
	${PROJECT_SOURCE_DIR}/src/net/coder/tinypb_coder.cc
	${PROJECT_SOURCE_DIR}/src/net/coder/tinypb_protocol.cc

	${PROJECT_SOURCE_DIR}/src/net/rpc/rpc_dispatcher.cc
	${PROJECT_SOURCE_DIR}/src/net/rpc/rpc_controller.cc
	${PROJECT_SOURCE_DIR}/src/net/rpc/rpc_channel.cc

)

# test目录源文件
set(SOURCE_FILE_TEST
	${PROJECT_SOURCE_DIR}/test/order.pb.cc
)

# 生成库文件
# add_library(mrpc_common STATIC ${SOURCE_FILE_COMMON})

# 添加可执行文件和链接库
add_executable(mrpc 
    ${SOURCE_FILE_COMMON} 
    ${SOURCE_FILE_NET} 
    ${PROJECT_SOURCE_DIR}/main.cc
)
target_link_libraries(mrpc ${LIBS})


## 测试用例
# test_log
add_executable(log_test 
	${SOURCE_FILE_COMMON} 
	${PROJECT_SOURCE_DIR}/test/test_log.cc
)
target_link_libraries(log_test ${LIBS})

# test_eventloop
add_executable(eventloop_test
	${SOURCE_FILE_COMMON} 
	${SOURCE_FILE_NET}
	${PROJECT_SOURCE_DIR}/test/test_eventloop.cc
)
target_link_libraries(eventloop_test ${LIBS})

# test_buffer
add_executable(buffer_test
	${SOURCE_FILE_COMMON} 
	${SOURCE_FILE_NET}
	${PROJECT_SOURCE_DIR}/test/test_buffer.cc
)
target_link_libraries(buffer_test ${LIBS})

# test_timer
add_executable(timer_test
	${SOURCE_FILE_COMMON} 
	${SOURCE_FILE_NET}
	${PROJECT_SOURCE_DIR}/test/test_timer.cc
)
target_link_libraries(timer_test ${LIBS})

# test_tcp
add_executable(tcp_test
	${SOURCE_FILE_COMMON} 
	${SOURCE_FILE_NET}
	${PROJECT_SOURCE_DIR}/test/test_tcp.cc
)
target_link_libraries(tcp_test ${LIBS})

# test_client
add_executable(client_test
	${SOURCE_FILE_COMMON} 
	${SOURCE_FILE_NET}
	${PROJECT_SOURCE_DIR}/test/test_client.cc
)
target_link_libraries(client_test ${LIBS})

# test_rpc_server
add_executable(rpc_server_test
	${SOURCE_FILE_COMMON} 
	${SOURCE_FILE_NET}
	${SOURCE_FILE_TEST}
	${PROJECT_SOURCE_DIR}/test/test_rpc_server.cc
)
target_link_libraries(rpc_server_test ${LIBS})

# test_rpc_client
add_executable(rpc_client_test
	${SOURCE_FILE_COMMON} 
	${SOURCE_FILE_NET}
	${SOURCE_FILE_TEST}
	${PROJECT_SOURCE_DIR}/test/test_rpc_client.cc
)
target_link_libraries(rpc_client_test ${LIBS})