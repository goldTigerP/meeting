#include "gui/main_window.h"
#include <QApplication>

int main(int argc, char **argv) {
  auto app = QApplication(argc, argv);
  auto window = MainWindow();
  window.show();
  app.exec();
  return 0;
}