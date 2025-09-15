#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

class MeetingApp : public QWidget {
  Q_OBJECT

public:
  MeetingApp(QWidget *parent = nullptr) : QWidget(parent) {
    setWindowTitle("会议管理系统");
    setMinimumSize(600, 400);

    setupUI();
    connectSignals();
  }

private slots:
  void onStartMeetingClicked() {
    QString meetingName = meetingNameEdit->text().trimmed();
    if (meetingName.isEmpty()) {
      QMessageBox::warning(this, "警告", "请输入会议名称！");
      return;
    }

    QString info =
        QString("已开始会议: %1\n时间: %2\n")
            .arg(meetingName)
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    meetingLog->append(info);
    meetingNameEdit->clear();
  }

  void onEndMeetingClicked() {
    meetingLog->append(
        QString("会议已结束 - %1\n")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
  }

  void onClearLogClicked() { meetingLog->clear(); }

private:
  void setupUI() {
    auto *mainLayout = new QVBoxLayout(this);

    // 标题
    auto *titleLabel = new QLabel("会议管理系统");
    titleLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #2c3e50; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // 会议输入区域
    auto *inputLayout = new QHBoxLayout();
    auto *nameLabel = new QLabel("会议名称:");
    meetingNameEdit = new QLineEdit();
    meetingNameEdit->setPlaceholderText("请输入会议名称...");

    inputLayout->addWidget(nameLabel);
    inputLayout->addWidget(meetingNameEdit);

    // 按钮区域
    auto *buttonLayout = new QHBoxLayout();
    startButton = new QPushButton("开始会议");
    endButton = new QPushButton("结束会议");
    clearButton = new QPushButton("清空日志");

    startButton->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; padding: 8px "
        "16px; border: none; border-radius: 4px; } QPushButton:hover { "
        "background-color: #229954; }");
    endButton->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; padding: 8px "
        "16px; border: none; border-radius: 4px; } QPushButton:hover { "
        "background-color: #c0392b; }");
    clearButton->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; padding: 8px "
        "16px; border: none; border-radius: 4px; } QPushButton:hover { "
        "background-color: #7f8c8d; }");

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(endButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();

    // 日志区域
    auto *logLabel = new QLabel("会议日志:");
    meetingLog = new QTextEdit();
    meetingLog->setReadOnly(true);
    meetingLog->setStyleSheet(
        "QTextEdit { border: 1px solid #bdc3c7; border-radius: 4px; padding: "
        "8px; background-color: #f8f9fa; }");

    // 添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(logLabel);
    mainLayout->addWidget(meetingLog);

    // 设置整体样式
    setStyleSheet("QWidget { background-color: #ecf0f1; } QLabel { color: "
                  "#2c3e50; font-weight: bold; }");
  }

  void connectSignals() {
    connect(startButton, &QPushButton::clicked, this,
            &MeetingApp::onStartMeetingClicked);
    connect(endButton, &QPushButton::clicked, this,
            &MeetingApp::onEndMeetingClicked);
    connect(clearButton, &QPushButton::clicked, this,
            &MeetingApp::onClearLogClicked);
    connect(meetingNameEdit, &QLineEdit::returnPressed, this,
            &MeetingApp::onStartMeetingClicked);
  }

  QLineEdit *meetingNameEdit;
  QPushButton *startButton;
  QPushButton *endButton;
  QPushButton *clearButton;
  QTextEdit *meetingLog;
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MeetingApp window;
  window.show();

  return app.exec();
}

#include "main.moc"
