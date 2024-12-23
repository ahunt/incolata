#include "connectiondialog.h"
#include "ui_connectiondialog.h"

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QPushButton>

#include "libp8020/libp8020.h"

ConnectionDialog::ConnectionDialog(QWidget* aParent)
  : QDialog(aParent)
  , mUI(new Ui::ConnectionDialog)
  , mPortLoaderThread(new PortLoaderThread)
  , mModel(new PortListModel(this))
{
  mUI.get()->setupUi(this);

  mUI.get()->buttonBox->button(QDialogButtonBox::Ok)->setText("Connect");
  mUI.get()->appWarningIcon->setPixmap(
    QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32)));
  mUI.get()->testWarningIcon->setPixmap(
    QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32)));

  mUI.get()->deviceComboBox->setModel(mModel);

  connect(
    this, &QDialog::finished, this, &ConnectionDialog::doEmitFinishedSignals);

  connect(mPortLoaderThread,
          &PortLoaderThread::portsReceived,
          mModel,
          &PortListModel::updatePorts);
  mPortLoaderThread->start();
}

ConnectionDialog::~ConnectionDialog() {}

void
ConnectionDialog::doEmitFinishedSignals(const int result)
{
  // TODO: make this robust against race conditions when devices disappear or
  // appear.
  mPortLoaderThread->mExit = true;
  if (result == QDialog::Accepted) {
    auto device =
      mModel->deviceAtIndex(mUI.get()->deviceComboBox->currentIndex());
    emit requestedConnectionToDevice(device);
  }
}

PortListModel::PortListModel(QObject* aParent)
  : QAbstractListModel(aParent)
  , mPorts(nullptr)
{
}

PortListModel::~PortListModel()
{
  if (this->mPorts != nullptr) {
    p8020_port_list_free(this->mPorts);
    this->mPorts = nullptr;
  }
}
int
PortListModel::rowCount(const QModelIndex&) const
{
  if (this->mPorts == nullptr) {
    return 0;
  }
  int i = p8020_port_list_count(this->mPorts);
  return i;
}

void
PortListModel::updatePorts(P8020PortList* aPorts)
{
  beginResetModel();
  if (this->mPorts != nullptr) {
    p8020_port_list_free(this->mPorts);
  }
  this->mPorts = aPorts;
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

  char* const name = p8020_port_list_port_name(this->mPorts, index.row());

  QString result;
  if (p8020_port_list_port_type(this->mPorts, index.row()) ==
      P8020PortType::Usb) {
    P8020UsbPortInfo* info =
      p8020_port_list_usb_port_info(this->mPorts, index.row());
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

QString
PortListModel::deviceAtIndex(const size_t index)
{
  char* const name = p8020_port_list_port_name(this->mPorts, index);
  QString result = QString(name);
  p8020_string_free(name);
  return result;
}

PortLoaderThread::PortLoaderThread(QObject* aParent)
  : QThread(aParent)
  , mExit(false)
{
  connect(this, &QThread::finished, this, &QThread::deleteLater);
}

void
PortLoaderThread::run()
{
  while (true) {
    if (mExit) {
      break;
    }
    P8020PortList* const ports = p8020_ports_list(true);
    if (mExit) {
      p8020_port_list_free(ports);
      break;
    }
    emit portsReceived(ports);
    msleep(1000);
  }
}
