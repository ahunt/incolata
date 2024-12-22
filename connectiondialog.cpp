#include "connectiondialog.h"
#include "ui_connectiondialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ConnectionDialog)
{
  ui.get()->setupUi(this);

  ui.get()->buttonBox->button(QDialogButtonBox::Ok)->setText("Connect");
  ui.get()->appWarningIcon->setPixmap(
    QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32)));
  ui.get()->testWarningIcon->setPixmap(
    QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32)));
}

ConnectionDialog::~ConnectionDialog() {}
