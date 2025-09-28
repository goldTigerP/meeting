#include "login_window.h"
#include "main_window.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QPixmap>
#include <QScreen>
#include <QSpacerItem>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextStream>

LoginWindow::LoginWindow() : QWidget() {
  setupUI();
  setupStyles();

  // 设置窗口属性
  setWindowTitle("会议系统 - 用户登录");
  setFixedSize(400, 350);
  setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

  // 居中显示
  QRect screenGeometry = QApplication::primaryScreen()->geometry();
  move(screenGeometry.center() - rect().center());

  // 加载历史昵称
  loadNicknameHistory();
}

void LoginWindow::setupUI() {
  // 创建主布局
  mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(25);
  mainLayout->setContentsMargins(40, 40, 40, 40);

  // 表单区域
  formLayout = new QFormLayout();
  formLayout->setSpacing(20);
  formLayout->setLabelAlignment(Qt::AlignRight);

  // 昵称输入（使用可编辑的组合框）
  nicknameCombo = new QComboBox();
  nicknameCombo->setEditable(true);
  nicknameCombo->setMaxCount(10); // 最多保存10个历史昵称
  nicknameCombo->lineEdit()->setPlaceholderText("请输入您的昵称...");
  nicknameCombo->lineEdit()->setMaxLength(20);

  // 添加到表单布局
  formLayout->addRow("昵称:", nicknameCombo);

  // 按钮区域
  buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing(15);

  loginButton = new QPushButton("进入");
  quitButton = new QPushButton("退出");

  loginButton->setDefault(true);
  //   loginButton->setFixedHeight(40);
  //   quitButton->setFixedHeight(40);

  buttonLayout->addStretch();
  buttonLayout->addWidget(quitButton);
  buttonLayout->addWidget(loginButton);

  // 添加到主布局
  mainLayout->addLayout(formLayout);
  mainLayout->addSpacing(30);
  mainLayout->addLayout(buttonLayout);
  mainLayout->addStretch();

  // 连接信号和槽
  connect(loginButton, &QPushButton::clicked, this,
          &LoginWindow::onLoginClicked);
  connect(quitButton, &QPushButton::clicked, this,
          &LoginWindow::onCancelClicked);
  connect(nicknameCombo->lineEdit(), &QLineEdit::textChanged, this,
          &LoginWindow::onNicknameChanged);
  connect(nicknameCombo->lineEdit(), &QLineEdit::returnPressed, this,
          &LoginWindow::onLoginClicked);
}

void LoginWindow::setupStyles() {
  setStyleSheet(
      "LoginWindow {"
      "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
      "                                stop:0 #f0f2f5, stop:1 #e8eaf0);"
      "    border-radius: 10px;"
      "}"

      "#titleLabel {"
      "    font-size: 26px;"
      "    font-weight: bold;"
      "    color: #2c3e50;"
      "    margin-bottom: 5px;"
      "}"

      "#welcomeLabel {"
      "    font-size: 16px;"
      "    color: #34495e;"
      "    margin-bottom: 5px;"
      "}"

      "#instructionLabel {"
      "    font-size: 14px;"
      "    color: #7f8c8d;"
      "    margin-bottom: 10px;"
      "}"

      "QComboBox {"
      "    padding: 12px 15px;"
      "    border: 2px solid #ddd;"
      "    border-radius: 8px;"
      "    font-size: 15px;"
      "    background-color: white;"
      "    min-height: 25px;"
      "    min-width: 250px;"
      "}"

      "QComboBox:focus {"
      "    border-color: #3498db;"
      "    outline: none;"
      "}"

      "QComboBox::drop-down {"
      "    border: none;"
      "    width: 30px;"
      "}"

      "QComboBox::down-arrow {"
      "    image: none;"
      "    border: 2px solid #7f8c8d;"
      "    width: 6px; height: 6px;"
      "    border-top: none;"
      "    border-right: none;"
      "    transform: rotate(-45deg);"
      "}"

      "QPushButton {"
      "    padding: 10px 25px;"
      "    border: none;"
      "    border-radius: 8px;"
      "    font-size: 15px;"
      "    font-weight: bold;"
      "    min-width: 90px;"
      "}"

      "QPushButton#loginButton {"
      "    background-color: #27ae60;"
      "    color: white;"
      "}"

      "QPushButton#loginButton:hover {"
      "    background-color: #229954;"
      "}"

      "QPushButton#quitButton {"
      "    background-color: #95a5a6;"
      "    color: white;"
      "}"

      "QPushButton#quitButton:hover {"
      "    background-color: #7f8c8d;"
      "}"

      "QLabel {"
      "    color: #2c3e50;"
      "}");

  // 设置按钮的对象名称以应用样式
  loginButton->setObjectName("loginButton");
  quitButton->setObjectName("quitButton");
}

void LoginWindow::onLoginClicked() {
  QString nickname = nicknameCombo->currentText().trimmed();

  // 验证输入
  if (nickname.isEmpty()) {
    QMessageBox::warning(this, "输入错误", "请输入您的昵称！");
    nicknameCombo->setFocus();
    return;
  }

  if (nickname.length() > 20) {
    QMessageBox::warning(this, "输入错误", "昵称不能超过20个字符！");
    nicknameCombo->setFocus();
    return;
  }

  // 保存昵称
  saveNickname(nickname);
  m_currentNickname = nickname;

  showMainWindow();
}

void LoginWindow::onCancelClicked() { qApp->quit(); }

void LoginWindow::onNicknameChanged(const QString &text) {
  // 实时验证昵称长度
  if (text.length() > 20) {
    nicknameCombo->lineEdit()->setText(text.left(20));
  }

  // 启用/禁用登录按钮
  loginButton->setEnabled(!text.trimmed().isEmpty());
}

void LoginWindow::showMainWindow() {
  // 创建并显示主窗口
  m_mainWindow = new MainWindow(m_currentNickname);
  m_mainWindow->setWindowTitle(QString("会议系统"));
  m_mainWindow->show();

  // 隐藏登录窗口
  this->hide();

  // 当主窗口关闭时，也关闭整个应用
  connect(m_mainWindow, &MainWindow::destroyed, qApp, &QApplication::quit);
}

void LoginWindow::loadNicknameHistory() {
  QStringList nicknames = loadNicknamesFromFile();

  // 添加到组合框
  for (const QString &nickname : nicknames) {
    if (!nickname.trimmed().isEmpty()) {
      nicknameCombo->addItem(nickname.trimmed());
    }
  }

  // 如果有历史记录，设置第一个为当前选择
  if (nicknameCombo->count() > 0) {
    nicknameCombo->setCurrentIndex(0);
  }

  qDebug() << "加载了" << nicknameCombo->count() << "个历史昵称";
}

void LoginWindow::saveNickname(const QString &nickname) {
  if (nickname.trimmed().isEmpty()) {
    return;
  }

  QStringList nicknames = loadNicknamesFromFile();

  // 移除重复项
  nicknames.removeAll(nickname);

  // 添加到开头
  nicknames.prepend(nickname);

  // 限制最多保存10个
  while (nicknames.size() > 10) {
    nicknames.removeLast();
  }

  // 保存到文件
  saveNicknamesToFile(nicknames);

  qDebug() << "保存昵称:" << nickname;
}

QString LoginWindow::getConfigFilePath() {
  // 获取应用程序所在目录
  QString appDir = QApplication::applicationDirPath();
  return QDir(appDir).filePath("user_config.txt");
}

QStringList LoginWindow::loadNicknamesFromFile() {
  QStringList nicknames;
  QString configPath = getConfigFilePath();

  QFile file(configPath);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    in.setCodec("UTF-8"); // 确保支持中文

    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      if (!line.isEmpty() && !nicknames.contains(line)) {
        nicknames.append(line);
      }
    }
    file.close();

    qDebug() << "从文件加载昵称:" << configPath << "共" << nicknames.size()
             << "个";
  } else {
    qDebug() << "配置文件不存在或无法读取:" << configPath;
  }

  return nicknames;
}

void LoginWindow::saveNicknamesToFile(const QStringList &nicknames) {
  QString configPath = getConfigFilePath();

  // 确保目录存在
  QDir dir = QFileInfo(configPath).dir();
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  QFile file(configPath);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out.setCodec("UTF-8"); // 确保支持中文

    for (const QString &nickname : nicknames) {
      if (!nickname.trimmed().isEmpty()) {
        out << nickname << "\n";
      }
    }
    file.close();

    qDebug() << "保存昵称到文件:" << configPath << "共" << nicknames.size()
             << "个";
  } else {
    qDebug() << "无法写入配置文件:" << configPath;
  }
}