#include "protocol.h"

#include <QString>

#include "libp8020/libp8020.h"

Protocol::~Protocol()
{
  switch (tag) {
    case Protocol::BUILTIN_CONFIG:
      break;
    case Protocol::BUILTIN_CONFIG_ID:
      delete builtinConfigID;
      break;
  }
}

QString
Protocol::id() const
{
  switch (tag) {
    case Protocol::BUILTIN_CONFIG: {
      char* const id = p8020_test_config_id(builtinConfig);
      const QString result = id;
      p8020_string_free(id);
      return result;
    }
    case Protocol::BUILTIN_CONFIG_ID:
      return *builtinConfigID;
  }
  assert(false && "unreachable");
  return QString("UNREACHABLE CODE REACHED");
}
