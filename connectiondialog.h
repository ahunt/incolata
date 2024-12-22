#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

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

#endif // CONNECTIONDIALOG_H
