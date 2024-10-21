#ifndef TESTWORKER_H
#define TESTWORKER_H

#include <QObject>

struct Device;
struct TestConfig;

class TestWorker : public QObject
{
  Q_OBJECT

public:
  TestWorker(Device* device, QObject* parent = 0);

private:
  Device* device;

public slots:
  // Runs test with testConfig. runTest assumes ownership of testConfig, and
  // will free it if/when needed.
  void runTest(TestConfig* testConfig);
};

#endif // TESTWORKER_H
