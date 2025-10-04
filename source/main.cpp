#include "gui/discovery_test_window.h"
#include "gui/login_window.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class TestLauncher : public QWidget {
  Q_OBJECT
public:
  TestLauncher(QWidget *parent = nullptr) : QWidget(parent) {
    setWindowTitle("Meeting App - 测试启动器");
    setFixedSize(400, 200);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("选择要测试的功能:", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; margin: 20px;");

    QPushButton *discoveryBtn = new QPushButton("设备发现功能测试", this);
    QPushButton *loginBtn = new QPushButton("登录窗口 (原功能)", this);

    discoveryBtn->setMinimumHeight(40);
    loginBtn->setMinimumHeight(40);

    layout->addWidget(titleLabel);
    layout->addWidget(discoveryBtn);
    layout->addWidget(loginBtn);
    layout->addStretch();

    connect(discoveryBtn, &QPushButton::clicked, [this]() {
      DiscoveryTestWindow *testWindow = new DiscoveryTestWindow();
      testWindow->setAttribute(Qt::WA_DeleteOnClose);
      testWindow->show();
      this->hide();
    });

    connect(loginBtn, &QPushButton::clicked, [this]() {
      LoginWindow *loginWindow = new LoginWindow();
      loginWindow->setAttribute(Qt::WA_DeleteOnClose);
      loginWindow->show();
      this->hide();
    });
  }
};

#include "main.moc"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  // 设置应用程序信息
  app.setApplicationName("Meeting Management System");
  app.setApplicationVersion("1.0.0");
  app.setOrganizationName("pjh-test");

  // 设置应用程序图标和样式
  app.setQuitOnLastWindowClosed(true);

  qInfo() << "启动 - 使用qDebug宏";

  // 创建并显示测试启动器
  TestLauncher launcher;
  launcher.show();

  qInfo() << "退出 - 使用qDebug宏";

  return app.exec();
}