#ifndef TESTWORKER_H
#define TESTWORKER_H

#include <QObject>
#include "incolata_common.h"

struct P8020Device;

class TestWorker : public QObject
{
  Q_OBJECT

public:
  TestWorker(P8020Device* device, QObject* parent = 0);

private:
  P8020Device* mDevice;

public slots:
  // Runs test with testConfig. runTest assumes ownership of testConfig, and
  // will free it if/when needed.
  void runTest(TestConfig* const config,
               void* const callback,
               void* const cb_data,
               const QString& specimen,
               const QString& subject,
               const QString& protocol);
};

#endif // TESTWORKER_H
