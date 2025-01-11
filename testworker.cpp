#include "testworker.h"
#include "libp8020/libp8020.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>

TestWorker::TestWorker(P8020Device* const aDevice, QObject* const aParent)
  : QObject(aParent)
  , mDevice(aDevice)
{
}

void
TestWorker::runTest(TestConfig* const aTestConfig,
                    const TestCallback aCallback,
                    void* const aCallbackData,
                    const QString& aSpecimen,
                    const QString& aSubject,
                    const QString& aComment,
                    const QString& aProtocol)
{
  // Opening the file before running the test is probably unnecessary, but it
  // helps me spot mistakes with the file path or whatever before running a
  // test.
  QFile testLog(QDir::homePath() + "/fit_testing/ftl_log.csv");
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

  P8020TestResult* result =
    p8020_device_run_test(mDevice, aTestConfig, aCallback, aCallbackData);

  emit testCompleted();
  if (result == nullptr) {
    return;
  }

  auto date = QDateTime::currentDateTime().toString("yyyy_MM_dd");
  for (size_t i = 0; i < result->fit_factors_length; i += 12) {
    // TODO: make this robust against non-ASCII specimen and subjects.
    stream << aSpecimen << ",";
    for (size_t j = i; j < i + 12; j++) {
      if (j >= result->fit_factors_length) {
        stream << ",";
        continue;
      }
      double ff = result->fit_factors[j];
      if (ff >= 10) {
        stream << QString::number(ff, 'f', 0);
      } else {
        stream << QString::number(ff, 'f', 1);
      }
      stream << ",";
    }
    stream << aComment << "," << aSubject << "," << date << "," << aProtocol
           << "\n";
  }
  testLog.close();

  // TODO: log raw data?
  // TODO: save to DB?

  p8020_test_result_free(result);
  p8020_test_config_free(aTestConfig);
}

TestWorker::~TestWorker()
{
  // Also ensures that we cleanly disconnect (exit external control mode).
  p8020_device_free(mDevice);
}
