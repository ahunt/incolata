#ifndef INCOLATA_COMMON_H
#define INCOLATA_COMMON_H

#include <QObject>

#include "protocol.h"

struct TestConfig;
struct TestNotification;

// TODO: wrap TestConfig* and P8020PortList* in QObjects with RAII instead. This
// is necessary to avoid the risk of leaks and crashes (the latter if a signal
// is connected to multiple slots).

// Needed for Qt 6.2, not needed for Qt 6.8 (it's unclear which intermediate
// versions it's needed for)
Q_DECLARE_OPAQUE_POINTER(TestConfig*);
Q_DECLARE_METATYPE(TestConfig*);

typedef void (*TestCallback)(const TestNotification*, void*);

Q_DECLARE_OPAQUE_POINTER(TestCallback);
Q_DECLARE_METATYPE(TestCallback);

Q_DECLARE_OPAQUE_POINTER(Protocol);
Q_DECLARE_METATYPE(Protocol);

// Must be called once during application startup.
inline static void
incolataRegisterMetaTypes()
{
  qRegisterMetaType<TestCallback>();
}

#endif // INCOLATA_COMMON_H
