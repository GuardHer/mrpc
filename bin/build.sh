#!/bin/bash

# 设置构建目录
BUILD_DIR="/home/guardher/code/project/mrpc/build"

# 检查构建目录是否存在
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory $BUILD_DIR does not exist."
    exit 1
fi

# 切换到构建目录
cd "$BUILD_DIR" || exit 1

# 检查是否提供了目标名称作为参数
if [ -z "$1" ]; then
    # 如果没有提供目标名称，则构建所有目标
    cmake --build . --config Debug -j 8
else
    # 如果提供了目标名称，则构建指定目标
    cmake --build . --target "$1" --config Debug -j 8
fi
