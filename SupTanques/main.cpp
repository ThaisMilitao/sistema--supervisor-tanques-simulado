#include "suptanques_main.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  SupTanquesMain w;
  w.show();
  return a.exec();
}
