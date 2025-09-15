# Meeting 项目开发指南

## 项目概述

这是一个基于 Qt5 和 CMake 的会议管理应用程序，展示了在 Ubuntu 系统下进行 Qt 开发的完整流程。

## 系统要求

- Ubuntu 20.04 或更高版本
- Qt5 开发包
- CMake 3.10 或更高版本
- GCC 编译器 (支持 C++17)

## 安装依赖

```bash
# 安装 Qt5 开发包
sudo apt update
sudo apt install qtbase5-dev qt5-qmake cmake build-essential

# 可选：安装 Qt Creator IDE
sudo apt install qtcreator
```

## 构建项目

### 方式一：使用构建脚本（推荐）

```bash
# 构建项目
./build.sh

# 构建并运行
./build.sh run

# 重新构建
./build.sh rebuild

# 清理构建文件
./build.sh clean

# 查看帮助
./build.sh help
```

### 方式二：使用 CMake 命令

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置项目
cmake ..

# 构建
make -j$(nproc)

# 运行
make run

# 清理
make clean

# 深度清理
make distclean
```

### 方式三：使用 Qt Creator

1. 打开 Qt Creator
2. 文件 -> 打开文件或项目
3. 选择项目根目录的 `CMakeLists.txt` 文件
4. 配置项目（选择 kit）
5. 构建并运行

## 项目结构

```
meeting/
├── CMakeLists.txt          # CMake 配置文件
├── build.sh               # 构建脚本
├── build.bat              # Windows 构建脚本
├── README.md              # 项目说明
├── LICENSE                # 许可证
├── source/                # 源代码目录
│   └── main.cpp           # 主程序文件
└── build/                 # 构建输出目录
    └── bin/               # 可执行文件目录
        └── meeting        # 生成的可执行文件
```

## CMake 配置特性

本项目的 CMakeLists.txt 包含以下特性：

- **自动 Qt MOC 处理**：自动处理 Qt 的元对象编译
- **跨平台支持**：支持 Windows、Linux 和 macOS
- **自定义目标**：
  - `make run`：构建并运行应用程序
  - `make rebuild`：重新构建项目
  - `make distclean`：深度清理构建文件
- **编译优化**：根据构建类型自动优化
- **详细信息输出**：显示 Qt 版本和编译器信息

## Qt 应用程序功能

当前的示例应用程序提供以下功能：

1. **会议管理界面**：简洁的图形用户界面
2. **会议创建**：输入会议名称并开始会议
3. **会议日志**：记录会议开始和结束时间
4. **样式支持**：使用 Qt 样式表美化界面
5. **信号槽机制**：演示 Qt 的事件处理机制

## 开发技巧

### 添加新的 Qt 组件

1. 在 CMakeLists.txt 中的 `find_package` 行添加新组件：
   ```cmake
   find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
   ```

2. 在 `target_link_libraries` 中链接新组件：
   ```cmake
   target_link_libraries(${PROJECT_NAME} PRIVATE 
       Qt5::Core 
       Qt5::Widgets
       Qt5::Network
   )
   ```

### 添加资源文件

1. 创建 `.qrc` 资源文件
2. 在 CMakeLists.txt 中添加：
   ```cmake
   qt5_add_resources(RESOURCES resources.qrc)
   add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCES})
   ```

### 添加 UI 文件

1. 创建 `.ui` 文件（使用 Qt Designer）
2. CMake 会自动处理 UI 文件（因为设置了 `CMAKE_AUTOUIC ON`）

## 调试

### 使用 GDB 调试

```bash
# 构建调试版本
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# 使用 GDB 调试
gdb ./bin/meeting
```

### 使用 Qt Creator 调试

1. 在 Qt Creator 中打开项目
2. 设置断点
3. 按 F5 开始调试

## 常见问题

### Qt5 找不到

```bash
# 检查 Qt5 安装
pkg-config --list-all | grep -i qt

# 设置 Qt5 路径（如果需要）
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt5
```

### 编译错误

1. 确保安装了所有依赖包
2. 检查 CMake 版本：`cmake --version`
3. 检查编译器版本：`gcc --version`

## 扩展建议

1. **添加数据库支持**：使用 Qt5::Sql 组件
2. **网络功能**：使用 Qt5::Network 进行网络通信
3. **多媒体支持**：使用 Qt5::Multimedia 处理音视频
4. **单元测试**：添加 Qt5::Test 组件进行测试
5. **国际化**：添加多语言支持

## 相关资源

- [Qt 官方文档](https://doc.qt.io/)
- [CMake 官方文档](https://cmake.org/documentation/)
- [Qt5 CMake 手册](https://doc.qt.io/qt-5/cmake-manual.html)