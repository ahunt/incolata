#include "testworker.h"
#include "libp8020/libp8020.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>

TestWorker::TestWorker(Device* device, QObject* parent)
  : QObject(parent)
  , device(device)
{
}

void
TestWorker::runTest(TestConfig* testConfig,
                    const QString& specimen,
                    const QString& subject,
                    const QString& protocol)
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

  TestResult* result = device_run_test(device, testConfig);

  auto date = QDate().toString("yyyy_MM_dd");
  for (size_t i = 0; i < result->exercise_count; i += 12) {
    // TODO: make this robust against non-ASCII specimen and subjects.
    stream << specimen << ",";
    // TODO: think of a less ugly way to structure this, once we've had some
    // more coffee.
    for (size_t j = i; j < i + 12; j++) {
      if (j >= result->exercise_count) {
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
    stream << "," << subject << "," << date << "," << protocol << "\n";
  }
  testLog.close();

  // TODO: log raw data?
  // TODO: save to DB?

  // TODO: p8020_test_result_free(result);
  // TODO: p8020_test_config_free(testConfig);
}
