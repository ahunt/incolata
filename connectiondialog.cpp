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
  , mModel0(new PortListModel(this, /* aIsSecondDevice = */ false))
  , mModel1(new PortListModel(this, /* aIsSecondDevice = */ true))
{
  mUI->setupUi(this);

  mUI->buttonBox->button(QDialogButtonBox::Ok)->setText("Connect");
  mUI->appWarningIcon->setPixmap(
    QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32)));
  mUI->testWarningIcon->setPixmap(
    QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32)));

  mUI->device0ComboBox->setModel(mModel0);
  mUI->device1ComboBox->setModel(mModel1);

  connect(
    this, &QDialog::finished, this, &ConnectionDialog::doEmitFinishedSignals);

  connect(mPortLoaderThread,
          &PortLoaderThread::portsReceived,
          mModel0,
          &PortListModel::updatePorts);
  connect(mPortLoaderThread,
          &PortLoaderThread::portsReceived,
          mModel1,
          &PortListModel::updatePorts);
  mPortLoaderThread->start();

  connect(mPortLoaderThread,
          &PortLoaderThread::portsCountUpdated,
          this,
          &ConnectionDialog::portsCountUpdated);
}

ConnectionDialog::~ConnectionDialog() {}

void
ConnectionDialog::doEmitFinishedSignals(const int result)
{
  // TODO: make this robust against race conditions when devices disappear or
  // appear.
  mPortLoaderThread->mExit = true;
  if (result == QDialog::Accepted) {
    auto device0 = mModel0->deviceAtIndex(mUI->device0ComboBox->currentIndex());
    assert(device0.has_value() && "Device 0 must always be valid");

    // TODO: check for device 1 too.
    emit requestedConnectionToDevice(device0.value());
  }
}

void
ConnectionDialog::portsCountUpdated(const int aCount)
{
  mUI->device1ComboBox->setEnabled(aCount > 1);
}

PortListModel::PortListModel(QObject* aParent, bool aIsSecondDevice)
  : QAbstractListModel(aParent)
  , mIsSecondDevice(aIsSecondDevice)
  , mPorts()
{
}

PortListModel::~PortListModel() {}

int
PortListModel::rowCount(const QModelIndex&) const
{
  const size_t count = mPorts.count();
  if (mIsSecondDevice) {
    return count + 1;
  }
  return count;
}

void
PortListModel::updatePorts(const PortMap& aPorts)
{
  if (mPorts == aPorts) {
    // Avoid resetting the model if at all possible, as this resets the user's
    // selection.
    return;
  }

  beginResetModel();
  this->mPorts = aPorts;
  endResetModel();
}

QVariant
PortListModel::data(const QModelIndex& aIndex, const int aRole) const
{
  switch (aRole) {
    case Qt::DisplayRole:
      break;
    default:
      return QVariant();
  }

  size_t row = aIndex.row();
  if (mIsSecondDevice) {
    if (row == 0) {
      return "<Not Selected>";
    }
    row--;
  }

  return mPorts.values()[row].mDisplayName;
}

std::optional<QString>
PortListModel::deviceAtIndex(const size_t& aIndex) const
{
  size_t row = aIndex;
  if (mIsSecondDevice) {
    if (row == 0) {
      return std::nullopt;
    }
    row--;
  }
  return mPorts.values()[aIndex].mID;
}

PortLoaderThread::PortLoaderThread(QObject* aParent)
  : QThread(aParent)
  , mExit(false)
{
  connect(this, &QThread::finished, this, &QThread::deleteLater);
}

PortMap
transformPorts(const P8020PortList* const aPorts)
{
  PortMap out;
  size_t portCount = p8020_port_list_count(aPorts);
  for (size_t i = 0; i < portCount; i++) {
    PortInfo info(aPorts, i);
    out[info.mID] = info;
  }
  return out;
}

void
PortLoaderThread::run()
{
  while (true) {
    if (mExit) {
      break;
    }
    P8020PortList* const rawPorts = p8020_ports_list(true);
    PortMap ports = transformPorts(rawPorts);
    p8020_port_list_free(rawPorts);

    if (mExit) {
      break;
    }
    emit portsCountUpdated(ports.count());
    emit portsReceived(ports);
    msleep(1000);
  }
}

PortInfo::PortInfo() {}

PortInfo::PortInfo(const P8020PortList* const aPorts, const size_t& aIndex)
{
  char* const name = p8020_port_list_port_name(aPorts, aIndex);
  this->mID = QString(name);

  if (p8020_port_list_port_type(aPorts, aIndex) == P8020PortType::Usb) {
    P8020UsbPortInfo* info = p8020_port_list_usb_port_info(aPorts, aIndex);
    assert(info && "p8020_port_list_usb_port_info is expected to return "
                   "non-NULL result for P8020PortType::Usb");

    if (info->product) {
      this->mDisplayName = QString(info->product) + " <…" +
                           QString(info->serial_number).right(6) + ">";
    } else {
      this->mDisplayName =
        QString("USB Device #") + QString(info->serial_number);
    }
    if (info->manufacturer) {
      this->mDisplayName += " (" + QString(info->manufacturer) + ")";
    }
    this->mDisplayName += " [" + QString(name) + "]";
    p8020_usb_port_info_free(info);
  } else {
    this->mDisplayName = QString(name);
  }
  p8020_string_free(name);
}
