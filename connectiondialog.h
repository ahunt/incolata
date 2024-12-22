#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QAbstractListModel>
#include <QDialog>

struct P8020PortList;

namespace Ui {
  class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConnectionDialog(QWidget *parent = nullptr);
  ~ConnectionDialog();

private:
  std::unique_ptr<Ui::ConnectionDialog> ui;
};

class PortListModel : public QAbstractListModel
{
public:
  explicit PortListModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;

  void refreshDevices();

private:
  P8020PortList* ports;
};

#endif // CONNECTIONDIALOG_H
