#include "main_window.h"
#include <QDateTime>
#include <QHBoxLayout>

MainWindow::MainWindow(QString name)
    : QWidget(), testLabel(this), testButton(this), userName(name) {
  resize(600, 500);
  setupUI();
  initializeNetwork();
}

void MainWindow::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);

  // 测试按钮和标签
  testButton.setText("测试连接");
  testButton.setStyleSheet("QPushButton { background-color: #27ae60; color: "
                           "white; padding: 8px 16px; "
                           "border: none; border-radius: 4px; } "
                           "QPushButton:hover { background-color: #229954; }");

  testLabel.setStyleSheet(
      "QLabel { border: 1px solid #bdc3c7; border-radius: 4px; padding: 8px; "
      "background-color: #ffffff; min-height: 60px; }");
  testLabel.setAlignment(Qt::AlignCenter);
  testLabel.setText("点击'测试连接'按钮查看网络状态");

  // 添加到主布局
  mainLayout->addWidget(&testButton);
  mainLayout->addWidget(&testLabel);

  // 连接信号
  connect(&testButton, &QPushButton::clicked,
          [this]() { initializeNetwork(); });
}

void MainWindow::initializeNetwork() {
  ip = NetworkUtil::getPrimaryIPAddress();
  port = NetworkUtil::getSystemAssignedPort();
  testLabel.setText(QString("Ip: %1, port: %2").arg(ip).arg(port));
}
