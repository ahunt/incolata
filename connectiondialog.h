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

class ConnectionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConnectionDialog(QWidget *parent = nullptr);
  ~ConnectionDialog();

  void closeEvent(QCloseEvent* event) override;

signals:
  void requestedConnectionToDevice(const QString& aDevice);

private:
  std::unique_ptr<Ui::ConnectionDialog> mUI;
  // Owned by itself, self-destructs as and when needed.
  PortLoaderThread* mPortLoaderThread;
  PortListModel* mModel;

private slots:
  void doEmitFinishedSignals(const int result);
};

class PortListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit PortListModel(QObject* parent = nullptr);
  ~PortListModel();

  int rowCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;

  QString deviceAtIndex(const size_t index);

public slots:
  void updatePorts(P8020PortList* const);

private:
  P8020PortList* mPorts;
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
  void portsReceived(P8020PortList* const);
};

#endif // CONNECTIONDIALOG_H
