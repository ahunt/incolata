#ifndef PROTOCOLSMODEL_H
#define PROTOCOLSMODEL_H

#include <QAbstractListModel>

#include "libp8020/libp8020.h"

class QLabel;
struct Protocol;

class ProtocolsModel : public QAbstractListModel
{
public:
  explicit ProtocolsModel(QObject* aParent = nullptr);

  auto rowCount(const QModelIndex& aParent) const -> int override;
  auto data(const QModelIndex& aIndex, const int aRole) const -> QVariant override;

  // Return the protocol for aIndex.
  auto  protocol(const int& aIndex) -> QSharedPointer<Protocol> const;

private:
  const P8020TestConfigList mBuiltinConfigs;
};

#endif // PROTOCOLSMODEL_H
