#!/bin/bash

# Meeting 项目构建脚本
# 作者: 自动生成
# 版本: 1.0.0

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

# 打印彩色信息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查 Qt5 是否安装
check_qt5() {
    print_info "检查 Qt5 安装..."
    if ! pkg-config --exists Qt5Core Qt5Widgets; then
        print_error "Qt5 未安装或未找到！请安装 Qt5 开发包："
        echo "sudo apt install qtbase5-dev qt5-qmake"
        exit 1
    fi
    print_success "Qt5 已安装"
}

# 创建构建目录
setup_build_dir() {
    print_info "设置构建目录..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
}

# 配置项目
configure() {
    print_info "配置 CMake 项目..."
    if cmake ..; then
        print_success "CMake 配置完成"
    else
        print_error "CMake 配置失败"
        exit 1
    fi
}

# 构建项目
build() {
    print_info "构建项目..."
    if make -j$(nproc); then
        print_success "构建完成"
    else
        print_error "构建失败"
        exit 1
    fi
}

# 运行应用程序
run_app() {
    print_info "运行应用程序..."
    if [ -f "$BUILD_DIR/bin/meeting" ]; then
        "$BUILD_DIR/bin/meeting"
    else
        print_error "可执行文件不存在，请先构建项目"
        exit 1
    fi
}

# 清理构建文件
clean() {
    print_info "清理构建文件..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"/*
        print_success "清理完成"
    else
        print_warning "构建目录不存在"
    fi
}

# 显示帮助信息
show_help() {
    echo "Meeting 项目构建脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  build    构建项目（默认）"
    echo "  run      构建并运行项目"
    echo "  clean    清理构建文件"
    echo "  rebuild  重新构建项目"
    echo "  help     显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0              # 构建项目"
    echo "  $0 run          # 构建并运行"
    echo "  $0 clean        # 清理"
    echo "  $0 rebuild      # 重新构建"
}

# 主函数
main() {
    case "${1:-build}" in
        "build")
            check_qt5
            setup_build_dir
            configure
            build
            ;;
        "run")
            check_qt5
            setup_build_dir
            configure
            build
            run_app
            ;;
        "clean")
            clean
            ;;
        "rebuild")
            clean
            check_qt5
            setup_build_dir
            configure
            build
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"