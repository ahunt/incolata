#include "testworker.h"
#include "p8020a-rs/libp8020a.h"

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
