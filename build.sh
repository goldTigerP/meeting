#!/bin/bash
# Linux/macOS构建脚本

# 创建构建目录
mkdir -p build

# 进入构建目录
cd build

# 运行CMake配置
cmake ..

# 编译项目
make -j$(nproc)

echo "编译完成！可执行文件位于: build/bin/meeting"