#pragma once

#include <QApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

#include "network/discovery_service.h"

namespace meeting {
namespace gui {

class LoginWindow : public QWidget {
    Q_OBJECT

Q_SIGNALS:
    void getUserName(const QString& name);

public:
    explicit LoginWindow(QWidget* parent = nullptr);
    ~LoginWindow() = default;

private:
    void initializeUI();

private:
    QLabel* title_label_{nullptr};
    QLabel* username_label_{nullptr};
    QLineEdit* username_edit_{nullptr};
    QPushButton* login_button_{nullptr};
    QPushButton* exit_button_{nullptr};
};

}  // namespace gui
}  // namespace meeting