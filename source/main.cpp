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
#include <QSplitter>
#include <QGroupBox>
#include "player/openglvideoplayer.h"

class MeetingApp : public QWidget {
  Q_OBJECT

public:
  MeetingApp(QWidget *parent = nullptr) : QWidget(parent) {
    setWindowTitle("会议管理系统 - 视频会议版");
    setMinimumSize(900, 600);

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
  
  void onStartVideoClicked() {
    if (videoPlayer) {
      videoPlayer->startDemo();
      startVideoButton->setText("停止视频");
      startVideoButton->setStyleSheet(
          "QPushButton { background-color: #e74c3c; color: white; padding: 8px "
          "16px; border: none; border-radius: 4px; } QPushButton:hover { "
          "background-color: #c0392b; }");
      meetingLog->append(QString("视频演示已开始 - %1\n")
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    }
  }
  
  void onStopVideoClicked() {
    if (videoPlayer) {
      videoPlayer->stopDemo();
      startVideoButton->setText("开始视频");
      startVideoButton->setStyleSheet(
          "QPushButton { background-color: #3498db; color: white; padding: 8px "
          "16px; border: none; border-radius: 4px; } QPushButton:hover { "
          "background-color: #2980b9; }");
      meetingLog->append(QString("视频演示已停止 - %1\n")
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    }
  }
  
  void onVideoFrameUpdated() {
    // 视频帧更新回调
  }
  
  void onVideoSizeChanged(int width, int height) {
    meetingLog->append(QString("视频尺寸: %1x%2\n").arg(width).arg(height));
  }

private:
  void setupUI() {
    auto *mainLayout = new QVBoxLayout(this);

    // 标题
    auto *titleLabel = new QLabel("会议管理系统 - 视频会议版");
    titleLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #2c3e50; margin: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // 主内容区域 - 使用水平分割器
    auto *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // 左侧：视频区域
    auto *videoGroup = new QGroupBox("视频区域");
    auto *videoLayout = new QVBoxLayout(videoGroup);
    
    // 创建视频播放器
    videoPlayer = new OpenGLVideoPlayer();
    videoPlayer->setMinimumSize(480, 360);
    videoLayout->addWidget(videoPlayer);
    
    // 视频控制按钮
    auto *videoControlLayout = new QHBoxLayout();
    startVideoButton = new QPushButton("开始视频");
    startVideoButton->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; padding: 8px "
        "16px; border: none; border-radius: 4px; } QPushButton:hover { "
        "background-color: #2980b9; }");
    
    auto *videoInfoLabel = new QLabel("点击开始视频演示");
    videoInfoLabel->setStyleSheet("color: #7f8c8d; font-style: italic;");
    
    videoControlLayout->addWidget(startVideoButton);
    videoControlLayout->addWidget(videoInfoLabel);
    videoControlLayout->addStretch();
    
    videoLayout->addLayout(videoControlLayout);
    mainSplitter->addWidget(videoGroup);
    
    // 右侧：会议控制区域
    auto *controlGroup = new QGroupBox("会议控制");
    auto *controlLayout = new QVBoxLayout(controlGroup);

    // 会议输入区域
    auto *inputLayout = new QHBoxLayout();
    auto *nameLabel = new QLabel("会议名称:");
    meetingNameEdit = new QLineEdit();
    meetingNameEdit->setPlaceholderText("请输入会议名称...");

    inputLayout->addWidget(nameLabel);
    inputLayout->addWidget(meetingNameEdit);
    controlLayout->addLayout(inputLayout);

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
    controlLayout->addLayout(buttonLayout);

    // 日志区域
    auto *logLabel = new QLabel("会议日志:");
    meetingLog = new QTextEdit();
    meetingLog->setReadOnly(true);
    meetingLog->setMaximumHeight(200);
    meetingLog->setStyleSheet(
        "QTextEdit { border: 1px solid #bdc3c7; border-radius: 4px; padding: "
        "8px; background-color: #f8f9fa; }");

    controlLayout->addWidget(logLabel);
    controlLayout->addWidget(meetingLog);
    
    mainSplitter->addWidget(controlGroup);
    
    // 设置分割器比例 (视频区域:控制区域 = 2:1)
    mainSplitter->setSizes({600, 300});
    
    // 添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(mainSplitter);

    // 设置整体样式
    setStyleSheet("QWidget { background-color: #ecf0f1; } QLabel { color: "
                  "#2c3e50; font-weight: bold; } QGroupBox { font-weight: bold; "
                  "border: 2px solid #bdc3c7; border-radius: 5px; margin: 5px; } "
                  "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; }");
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
    
    // 视频播放器信号连接
    connect(startVideoButton, &QPushButton::clicked, this, [this]() {
      if (startVideoButton->text() == "开始视频") {
        onStartVideoClicked();
      } else {
        onStopVideoClicked();
      }
    });
    
    connect(videoPlayer, &OpenGLVideoPlayer::frameUpdated, this,
            &MeetingApp::onVideoFrameUpdated);
    connect(videoPlayer, &OpenGLVideoPlayer::videoSizeChanged, this,
            &MeetingApp::onVideoSizeChanged);
  }

  // 成员变量
  QLineEdit *meetingNameEdit;
  QPushButton *startButton;
  QPushButton *endButton;
  QPushButton *clearButton;
  QPushButton *startVideoButton;
  QTextEdit *meetingLog;
  OpenGLVideoPlayer *videoPlayer;
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MeetingApp window;
  window.show();

  return app.exec();
}

#include "main.moc"
