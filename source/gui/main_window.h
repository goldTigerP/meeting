#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "../util/network_util.h"
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

class MainWindow : public QWidget {
  Q_OBJECT
public:
  explicit MainWindow(QString name);

private:
  void setupUI();
  void initializeNetwork();

  // 原有控件
  QLabel testLabel;
  QPushButton testButton;

  QString userName;

  QString ip;
  quint16 port;
};

#endif
