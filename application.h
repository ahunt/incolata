#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class Application : public QApplication
{
public:
  Application(int& aArgc, char** aArgv);
};

#endif // APPLICATION_H
