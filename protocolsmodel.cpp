#include "protocolsmodel.h"

#include "protocol.h"

ProtocolsModel::ProtocolsModel(QObject* aParent)
  : QAbstractListModel(aParent)
  , mBuiltinConfigs(p8020_test_config_get_builtin())
{
}

int
ProtocolsModel::rowCount(const QModelIndex&) const
{
  return mBuiltinConfigs.count;
}

QVariant
ProtocolsModel::data(const QModelIndex& aIndex, const int aRole) const
{
  if (aRole != Qt::DisplayRole) {
    return QVariant();
  }

  const TestConfig* const config = mBuiltinConfigs.configs[aIndex.row()];

  char* const id = p8020_test_config_id(config);
  char* const name = p8020_test_config_name(config);

  QString result = QString("%2 [%1]").arg(id, name);

  p8020_string_free(id);
  p8020_string_free(name);

  return result;
}

QSharedPointer<Protocol>
ProtocolsModel::protocol(const int& aIndex) const
{
  // This will (obviously) change significantly once custom protocols are
  // supported.
  return QSharedPointer<Protocol>(new Protocol{
    .tag = Protocol::BUILTIN_CONFIG,
    .builtinConfig = mBuiltinConfigs.configs[aIndex],
  });
}
