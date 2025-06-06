#include "application.h"
#include "connectiondialog.h"

#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
  Application a(argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString& locale : uiLanguages) {
    const QString baseName = "ftl_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      a.installTranslator(&translator);
      break;
    }
  }

  ConnectionDialog d;
  QObject::connect(&d,
                   &ConnectionDialog::requestedConnectionToDevice,
                   &a,
                   &Application::connectToDevice);
  d.show();
  return a.exec();
}
