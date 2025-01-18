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

  int rowCount(const QModelIndex& aParent) const override;
  QVariant data(const QModelIndex& aIndex, const int aRole) const override;

  // Return the protocol for aIndex.
  QSharedPointer<Protocol> protocol(const int& aIndex) const;

private:
  const P8020TestConfigList mBuiltinConfigs;
};

#endif // PROTOCOLSMODEL_H
