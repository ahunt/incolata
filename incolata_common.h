#ifndef INCOLATA_COMMON_H
#define INCOLATA_COMMON_H

#include <QObject>

struct TestConfig;

// Needed for Qt 6.2, not needed for Qt 6.8 (it's unclear which intermediate
// versions it's needed for)
Q_DECLARE_OPAQUE_POINTER(const TestConfig*);
Q_DECLARE_METATYPE(const TestConfig*);

#endif // INCOLATA_COMMON_H
