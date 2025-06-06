#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include "incolata_common.h"

#include <QAbstractListModel>
#include <QDialog>
#include <QThread>

class QCloseEvent;

namespace Ui {
  class ConnectionDialog;
}

class PortLoaderThread;
class PortListModel;
struct PortInfo;
struct P8020PortList;

// PortMap is a map of port ID to PortInfo. QMap is used to ensure stable
// ordering for display purposes.
typedef QMap<QString, PortInfo> PortMap;

class ConnectionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConnectionDialog(QWidget *parent = nullptr);
  ~ConnectionDialog();

signals:
  void requestedConnectionToDevice(const QString& aDevice);

private:
  std::unique_ptr<Ui::ConnectionDialog> mUI;
  // Owned by itself, self-destructs as and when needed.
  PortLoaderThread* mPortLoaderThread;
  PortListModel* mModel0;
  PortListModel* mModel1;

private slots:
  void doEmitFinishedSignals(const int result);
  void portsCountUpdated(const int aCount);
};

class PortListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit PortListModel(QObject* parent = nullptr,
                         bool aIsSecondDevice = false);
  ~PortListModel();

  int rowCount(const QModelIndex&) const;
  QVariant data(const QModelIndex&, int) const;

  std::optional<QString> deviceAtIndex(const size_t&) const;

public slots:
  void updatePorts(const PortMap&);

private:
  bool mIsSecondDevice;
  PortMap mPorts;
};

// PortLoaderThread refreshes ports in the background.
// Callers must ensure that signal receivedPorts is connected prior to starting
// this thread.
class PortLoaderThread : public QThread
{
  Q_OBJECT

public:
  PortLoaderThread(QObject* parent = nullptr);
  void run() override;

  volatile bool mExit;

signals:
  void portsReceived(const PortMap&);
  void portsCountUpdated(const size_t);
};

struct PortInfo
{
  PortInfo();
  PortInfo(const P8020PortList* const, const size_t&);

  // Corresponds to path on *nix.
  QString mID;
  QString mDisplayName;

  bool operator==(const PortInfo&) const = default;
};

Q_DECLARE_METATYPE(PortInfo);

#endif // CONNECTIONDIALOG_H
