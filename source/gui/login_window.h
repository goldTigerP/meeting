#ifndef LOGIN_WINDOW_H
#define LOGIN_WINDOW_H

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow; // 前向声明

class LoginWindow : public QWidget {
  Q_OBJECT

public:
  LoginWindow();


private slots:
  void onLoginClicked();
  void onCancelClicked();
  void onNicknameChanged(const QString &text);

private:
  void setupUI();
  void setupStyles();
  void showMainWindow();

  // 昵称持久化相关
  void loadNicknameHistory();
  void saveNickname(const QString &nickname);
  QString getConfigFilePath();
  QStringList loadNicknamesFromFile();
  void saveNicknamesToFile(const QStringList &nicknames);

  QComboBox *nicknameCombo;

  QPushButton *loginButton;
  QPushButton *quitButton;

  // 布局
  QVBoxLayout *mainLayout;
  QFormLayout *formLayout;
  QHBoxLayout *buttonLayout;

  // 数据
  QString m_currentNickname;

  // 主窗口指针
  MainWindow *m_mainWindow{nullptr};
};

#endif // LOGIN_WINDOW_H