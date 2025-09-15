# Meeting Project

一个跨平台的C++项目，支持在Windows和Linux下编译运行。

## 构建要求

- CMake 3.10 或更高版本
- C++17 兼容的编译器
  - Linux: GCC 7+ 或 Clang 5+
  - Windows: Visual Studio 2017+ 或 MinGW

## 构建方法

### Linux/macOS

使用提供的构建脚本：
```bash
./build.sh
```

或者手动构建：
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### Windows

使用提供的构建脚本：
```cmd
build.bat
```

或者手动构建：
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## 运行程序

编译完成后，可执行文件将位于：
- Linux: `build/bin/meeting`
- Windows: `build/bin/Release/meeting.exe`

## 项目结构

```
meeting/
├── CMakeLists.txt    # CMake构建配置
├── build.sh          # Linux构建脚本
├── build.bat         # Windows构建脚本
├── source/           # 源代码目录
│   └── main.cpp
├── build/            # 构建输出目录
└── README.md         # 本文件
```

## CMake特性

- 支持Debug和Release构建模式
- 跨平台编译器配置
- 自动检测操作系统并应用相应设置
- 统一的输出目录结构