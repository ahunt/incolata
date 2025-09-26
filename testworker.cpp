#include "testworker.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>

#include "libp8020/libp8020.h"
#include "protocol.h"

TestWorker::TestWorker(P8020Device* const aDevice, QObject* const aParent)
  : QObject(aParent)
  , mDevice(aDevice)
{
}

QString
enquote(const QString& aIn)
{
  // Same as in libp8020: using an established CSV library would be the sensible
  // thing to do, but being sensible has no place here.
  bool hasQuotationMark = false;
  bool hasComma = false;
  for (auto it = aIn.cbegin(), end = aIn.cend(); it != end; it++) {
    if (*it == '"') {
      hasQuotationMark = true;
    } else if (*it == ',') {
      hasComma = true;
    }
  }

  if (!hasQuotationMark && !hasComma) {
    return QString(aIn);
  };
  QString out = aIn;
  if (hasQuotationMark) {
    out = out.replace('"', "\"\"");
  }
  return QString("\"%1\"").arg(out);
}

void
TestWorker::runTest(const QSharedPointer<Protocol> aProtocol,
                    const TestCallback aCallback,
                    void* const aCallbackData,
                    const QString& aSpecimen,
                    const QString& aSubject,
                    const QString& aComment)
{
  QDir logDir(QDir::homePath() + "/fit_testing/");
  if (!logDir.exists()) {
    qInfo() << "log directory does not exist, attempting to create it";
    // QDir does not offer an API to create the directory it points to, there's
    // either mkdir(someSubdir), or this.
    if (!logDir.mkpath(logDir.path())) {
      // TODO: introduce some kind of user-friendly error handling.
      qWarning() << "Unable to create log dir (" << logDir.path() << ")";
      return;
    }
  }

  QFile testLog(logDir.path() + "/ftl_log.csv");
  QIODeviceBase::OpenMode openOptions = QIODeviceBase::WriteOnly;
  if (testLog.exists()) {
    openOptions |= QIODeviceBase::Append;
  }
  if (!testLog.open(openOptions)) {
    // TODO: introduce some kind of user-friendly error handling.
    qWarning() << "Unable to open log (" << testLog.fileName()
               << "): " << testLog.errorString();
    return;
  }
  QTextStream stream(&testLog);

  const TestConfig* testConfig;
  switch (aProtocol->tag) {
    case Protocol::BUILTIN_CONFIG:
      testConfig = aProtocol->builtinConfig;
      break;
    case Protocol::BUILTIN_CONFIG_ID:
      assert(false && "not supported yet - needs to be moved into Protocol");
      break;
  }
  P8020TestResult* result =
    p8020_device_run_test(mDevice, testConfig, aCallback, aCallbackData);

  emit testCompleted();
  if (result == nullptr) {
    return;
  }

  auto date = QDateTime::currentDateTime().toString("yyyy_MM_dd");
  auto exerciseCount = p8020_test_result_get_exercise_count(result);
  for (size_t i = 0; i < exerciseCount; i += 12) {
    // TODO: make this robust against non-ASCII specimen and subjects.
    stream << enquote(aSpecimen) << ",";
    for (size_t j = i; j < i + 12; j++) {
      if (j >= exerciseCount) {
        stream << ",";
        continue;
      }
      double ff =
        p8020_test_result_get_fit_factor(result, /* device_id */ 0, j);
      if (ff >= 10) {
        stream << QString::number(ff, 'f', 0);
      } else {
        stream << QString::number(ff, 'f', 1);
      }
      stream << ",";
    }
    stream << enquote(aComment) << "," << enquote(aSubject) << "," << date
           << "," << aProtocol->id() << "\n";
  }
  testLog.close();

  // TODO: log raw data?
  // TODO: save to DB?

  p8020_test_result_free(result);
}

TestWorker::~TestWorker()
{
  // Also ensures that we cleanly disconnect (exit external control mode).
  p8020_device_free(mDevice);
}
