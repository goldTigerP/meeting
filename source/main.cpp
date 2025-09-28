#include "gui/login_window.h"
#include <QApplication>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("Meeting Management System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("pjh-test");
    
    // 设置应用程序图标和样式
    app.setQuitOnLastWindowClosed(true);

    // 创建并显示登录窗口
    LoginWindow loginWindow;
    loginWindow.show();

    return app.exec();
}