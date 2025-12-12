# 设置最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本
project(meeting VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器特定的编译选项
if(WIN32)
    # Windows特定配置
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    # 对于MSVC编译器
    if(MSVC)
        add_compile_options(/W4)  # 启用警告级别4
        add_compile_options(/utf-8)  # 使用UTF-8编码
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux特定配置
    add_compile_options(-Wall -Wextra -Wpedantic)  # 启用更多警告
endif()

# 设置构建类型的默认值
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Debug和Release模式的特殊设置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    if(WIN32 AND MSVC)
        add_compile_options(/Od)  # 禁用优化
    else()
        add_compile_options(-O0 -g)  # 禁用优化，启用调试信息
    endif()
else()
    add_definitions(-DNDEBUG)
    if(WIN32 AND MSVC)
        add_compile_options(/O2)  # 优化
    else()
        add_compile_options(-O2)  # 优化
    endif()
endif()

# 添加源文件目录
file(GLOB_RECURSE SOURCES "source/*.cpp" "source/*.h")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

# 链接系统库（如果需要）
if(WIN32)
    # Windows特定的库
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)  # 网络功能
elseif(UNIX AND NOT APPLE)
    # Linux特定的库
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread)  # 线程库
endif()

# 打印一些有用的信息
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")