#include "login_window.h"

#include <QDateTime>
#include <QFont>
#include <QGridLayout>

#include "util/common_define.h"

namespace meeting {
namespace gui {

LoginWindow::LoginWindow(QWidget* parent) : QWidget(parent) {
    initializeUI();
}

void LoginWindow::initializeUI() {
    setWindowTitle("Meeting - 登录");
    setFixedSize(300, 150);

    // 创建主布局
    auto* main_layout = new QVBoxLayout(this);

    auto* login_layout = new QHBoxLayout();
    username_label_    = new QLabel("用户名:", this);
    username_edit_     = new QLineEdit(this);
    username_edit_->setPlaceholderText("请输入您的用户名");
    username_edit_->setMaxLength(20);
    login_layout->addWidget(username_label_);
    login_layout->addWidget(username_edit_);
    main_layout->addLayout(login_layout);

    auto* button_layout = new QHBoxLayout();
    exit_button_        = new QPushButton("退出", this);
    login_button_       = new QPushButton("登录", this);
    button_layout->addWidget(exit_button_);
    button_layout->addWidget(login_button_);
    main_layout->addLayout(button_layout);

    connect(exit_button_, &QPushButton::pressed, [this]() { this->close(); });
    connect(login_button_, &QPushButton::pressed, [this]() {
        const auto name = this->username_edit_->displayText();
        if (name.isEmpty()) {
            return;
        }
        emit getUserName(name);
    });
}

}  // namespace gui
}  // namespace meeting