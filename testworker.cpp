#include "testworker.h"
#include "libp8020/libp8020.h"

TestWorker::TestWorker(Device* device, QObject* parent)
  : QObject(parent)
  , device(device)
{
}

void
TestWorker::runTest(TestConfig* testConfig)
{
  device_run_test(device, testConfig);

  // TODO: dump data to file (and, eventually, DB).

  // TODO: free testConfig.
}
