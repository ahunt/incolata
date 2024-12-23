#include "application.h"

#include "mainwindow.h"

Application::Application(int& aArgc, char** aArgv)
  : QApplication(aArgc, aArgv)
{
}

void
Application::connectToDevice(const QString& aPath)
{
  MainWindow* w = new MainWindow(aPath);
  w->show();
}
