#include "connectiondialog.h"
#include "ui_connectiondialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ConnectionDialog)
{
  ui->setupUi(this);

  ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Connect");
  ui->warningIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32)));
}

ConnectionDialog::~ConnectionDialog()
{
  delete ui;
}
