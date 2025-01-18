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
      char* const short_name = p8020_test_config_short_name(builtinConfig);
      const QString result = short_name;
      p8020_string_free(short_name);
      return result;
    }
    case Protocol::BUILTIN_CONFIG_ID:
      return *builtinConfigID;
  }
  assert(false && "unreachable");
  return QString("UNREACHABLE CODE REACHED");
}
