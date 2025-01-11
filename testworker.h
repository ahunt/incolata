#ifndef TESTWORKER_H
#define TESTWORKER_H

#include <QObject>
#include "incolata_common.h"

struct P8020Device;

class TestWorker : public QObject
{
  Q_OBJECT

public:
  TestWorker(P8020Device* const aDevice, QObject* const aParent = 0);

private:
  P8020Device* const mDevice;

public slots:
  // Runs test with testConfig. runTest assumes ownership of testConfig, and
  // will free it if/when needed.
  void runTest(TestConfig* const aConfig,
               const TestCallback aCallback,
               void* const aCallbackData,
               const QString& aSpecimen,
               const QString& aSubject,
               const QString& aComment,
               const QString& aProtocol);

signals:
  void testCompleted();
};

#endif // TESTWORKER_H
