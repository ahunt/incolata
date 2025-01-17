#ifndef PROTOCOLSMODEL_H
#define PROTOCOLSMODEL_H

#include "libp8020/libp8020.h"
#include <QAbstractListModel>

class QLabel;

class ProtocolsModel : public QAbstractListModel
{
public:
  explicit ProtocolsModel(QObject* aParent = nullptr);

  int rowCount(const QModelIndex& aParent) const override;
  QVariant data(const QModelIndex& aIndex, const int aRole) const override;

private:
  const P8020TestConfigList mBuiltinConfigs;
};

#endif // PROTOCOLSMODEL_H
