#include "connectiondialog.h"
#include "ui_connectiondialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include "libp8020/libp8020.h"

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

  PortListModel* model = new PortListModel(this);
  model->refreshDevices();
  ui.get()->deviceComboBox->setModel(model);
}

ConnectionDialog::~ConnectionDialog() {}

PortListModel::PortListModel(QObject* parent)
  : QAbstractListModel{ parent }
  , ports(NULL)
{
}

int
PortListModel::rowCount(const QModelIndex&) const
{
  if (this->ports == NULL) {
    return 0;
  }
  int i = p8020_port_list_count(this->ports);
  return i;
}

void
PortListModel::refreshDevices()
{
  P8020PortList* newPorts = p8020_ports_list(true);
  beginResetModel();
  if (this->ports != NULL) {
    p8020_port_list_free(this->ports);
  }
  this->ports = newPorts;
  endResetModel();
}

QVariant
PortListModel::data(const QModelIndex& index, int role) const
{
  switch (role) {
    case Qt::DisplayRole:
      break;
    default:
      return QVariant();
  }

  char* const name = p8020_port_list_port_name(this->ports, index.row());

  QString result;
  if (p8020_port_list_port_type(this->ports, index.row()) ==
      P8020PortType::Usb) {
    P8020UsbPortInfo* info =
      p8020_port_list_usb_port_info(this->ports, index.row());
    assert(info && "p8020_port_list_usb_port_info is expected to return "
                   "non-NULL result for P8020PortType::Usb");

    if (info->product) {
      result = QString(info->product) + " <…" + QString(info->serial_number).right(6) + ">";
    } else {
      result = QString("USB Device #") + QString(info->serial_number);
    }
    if (info->manufacturer) {
      result += " (" + QString(info->manufacturer) + ")";
    }
    result += " [" + QString(name) + "]";
    p8020_usb_port_info_free(info);
  } else {
    result = QString(name);
  }

  p8020_string_free(name);
  return result;
}
