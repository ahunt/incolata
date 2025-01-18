#ifndef PROTOCOL_H
#define PROTOCOL_H

struct TestConfig;
class QString;

// Represent a test protocol - may wrap a TestConfig or may just contain an ID.
struct Protocol
{
  enum
  {
    BUILTIN_CONFIG,
    BUILTIN_CONFIG_ID
  } tag;

  union
  {
    // Owned by libp8020 (static lifetime, should not be freed).
    const TestConfig* const builtinConfig;
    // libp8020 builtin config ID.
    const QString* builtinConfigID;
  };

  QString id() const;

  ~Protocol();
};

#endif // PROTOCOLSMODEL_H
