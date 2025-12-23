
#include <QApplication>
#include <QStyleFactory>

#include "core/network/discovery_service.h"
#include "gui/login_window.h"
#include "gui/style_sheet.h"
#include "util/common_define.h"

meeting::gui::LoginWindow* login_window;

void getUserName(const QString& name) {
    LOG_INFO << "get user name: " << name.toStdString();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Meeting");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PJH-dev");

    app.setStyle(QStyleFactory::create("Fusion"));
    app.setStyleSheet(meeting::gui::STYLE_SHEET);

    try {
        login_window = new meeting::gui::LoginWindow();
        login_window->show();
        QObject::connect(login_window, &meeting::gui::LoginWindow::getUserName, getUserName);

        // 运行应用程序事件循环
        int result = app.exec();

        LOG_INFO << "Application exiting with code: " << result;
        return result;

    } catch (const std::exception& e) {
        LOG_ERROR << "Application exception: " << e.what();
        return -1;
    }
}