cmake_minimum_required(VERSION 3.16)
project(mrpc)

# 设置c++标准
set(CMAKE_CXX_STANDARD 17)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O0 -Wno-unused-variable -pthread -ldl")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) # 设置可执行文件输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)    # 设置库文件输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/obj) # 设置*.o文件输出路径


# 设置头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    "/usr/local/include/google"   # protobuf头文件路径
    "/usr/local/include/tinyxml"  # tinyxml头文件路径
)

# 设置库文件路径
link_directories(
    "/usr/local/lib" # protobuf, tinyxml库文件路径
)

# 链接库
set(LIBS
    libprotobuf.a
    libtinyxml.a
)

# common目录源文件
set(SOURCE_FILE_COMMON
    ${PROJECT_SOURCE_DIR}/src/common/util.cc
    ${PROJECT_SOURCE_DIR}/src/common/log.cc
)

# net目录源文件
set(SOURCE_FILE_NET
    # 添加net目录中的源文件
)

# 添加可执行文件和链接库
add_executable(mrpc 
    ${SOURCE_FILE_COMMON} 
    ${SOURCE_FILE_NET} 
    ${PROJECT_SOURCE_DIR}/main.cc
)

# target_link_libraries(mrpc ${LIBS})
