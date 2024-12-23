#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class Application : public QApplication
{
public:
  Application(int& aArgc, char** aArgv);

public slots:
  void connectToDevice(const QString& aPath);
};

#endif // APPLICATION_H
