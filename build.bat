@echo off
REM Windows构建脚本

REM 创建构建目录
if not exist build mkdir build

REM 进入构建目录
cd build

REM 运行CMake配置（使用Visual Studio生成器）
cmake .. -G "Visual Studio 16 2019" -A x64

REM 编译项目
cmake --build . --config Release

echo 编译完成！可执行文件位于: build\bin\Release\meeting.exe
pause