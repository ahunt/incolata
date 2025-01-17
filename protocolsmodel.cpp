#include "protocolsmodel.h"

#include "libp8020/libp8020.h"

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

  char* const short_name = p8020_test_config_short_name(config);
  char* const name = p8020_test_config_name(config);

  QString result = QString("%2 [%1]").arg(short_name, name);

  p8020_string_free(short_name);
  p8020_string_free(name);

  return result;
}
