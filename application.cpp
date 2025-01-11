#include "application.h"

#include "incolata_common.h"
#include "mainwindow.h"

Application::Application(int& aArgc, char** aArgv)
  : QApplication(aArgc, aArgv)
{
  incolataRegisterMetaTypes();
}

void
Application::connectToDevice(const QString& aPath)
{
  MainWindow* w = new MainWindow(aPath);
  w->show();
}
