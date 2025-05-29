#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QChart>
#include <QLineEdit>
#include <QLineSeries>
#include <QLogValueAxis>
#include <QStatusBar>
#include <QThread>
#include <QValueAxis>

#include "config.h"
#include "exerciserowwidget.h"
#include "exercisesmodel.h"
#include "libp8020/libp8020.h"
#include "protocol.h"
#include "protocolsmodel.h"
#include "testworker.h"

static const qsizetype sRawSampleRange = 180;
static const qsizetype sAmbientSampleRange = 40;
static const qsizetype sSpecimenSampleRange = 60;

void
MainWindow::device_callback(const P8020DeviceNotification* notification,
                            void* cb_data)
{
  MainWindow* mw = static_cast<MainWindow*>(cb_data);
  switch (notification->tag) {
    case P8020DeviceNotification::Tag::Sample: {
      const double sample = notification->sample.particle_conc;
      emit mw->receivedRawSample(notification->sample.device_id, sample);
      if (sample < 100) {
        emit mw->renderRawSample(QString::number(sample, 'f', 2));
      } else {
        emit mw->renderRawSample(QString::number(sample, 'f', 0));
      }
      break;
    }
    case P8020DeviceNotification::Tag::ConnectionClosed:
      // TODO: handle loss of connection.
      break;
    case P8020DeviceNotification::Tag::DevicePropertiesAvailable:
      emit mw->refreshStatusBar();
      break;
  }
}

void
MainWindow::test_callback(const TestNotification* notification, void* cb_data)
{
  MainWindow* mw = static_cast<MainWindow*>(cb_data);
  switch (notification->tag) {
    case TestNotification::Tag::StateChange: {
      const TestState ts = notification->state_change._0;
      switch (ts.tag) {
        case TestState::Tag::StartedExercise:
          emit mw->exerciseChanged(uint(ts.started_exercise._0));
          break;
        default:
          // TODO: handle these.
          break;
      }
      break;
    }
    case TestNotification::Tag::ExerciseResult: {
      auto& result = notification->exercise_result;
      emit mw->ffUpdated(result.device_id, result.exercise, result.fit_factor, result.error);
      break;
    };
    case TestNotification::Tag::Sample: {
      auto& sample = notification->sample._0;
      switch (sample.sample_type) {
        case SampleType::AmbientSample:
        case SampleType::SpecimenSample:
          emit mw->receivedSample(sample.device_id,
                                  sample.sample_type,
                                  sample.exercise,
                                  sample.value);
          break;
        case SampleType::AmbientPurge:
        case SampleType::SpecimenPurge:
          break;
      }
      break;
    }
    case TestNotification::Tag::LiveFF: {
      auto& ff = notification->live_ff;
      emit mw->receivedLiveFF(
        ff.device_id, ff.exercise, ff.index, ff.fit_factor);
      break;
    }
    case TestNotification::Tag::InterimFF:
      auto& ff = notification->interim_ff;
      emit mw->receivedInterimFF(ff.device_id, ff.exercise, ff.fit_factor);
      break;
  }
}

void
MainWindow::processRawSample(const size_t aDeviceIndex, const double aSample)
{
  mUI->rawChartView->addDatapoint(aDeviceIndex, /*seriesIndex*/ 0, aSample);
}

void
MainWindow::processSample(const size_t aDeviceIndex,
                          const SampleType aSampleType,
                          const size_t aExercise,
                          const double aValue)
{
  switch (aSampleType) {
    case SampleType::AmbientSample:
      mUI->ambientSampleGraph->addDatapoint(aDeviceIndex, aExercise, aValue);
      break;
    case SampleType::SpecimenSample:
      mUI->specimenSampleGraph->addDatapoint(aDeviceIndex, aExercise, aValue);
      break;
    default:
      // TODO: handle remaining cases.
      break;
  }
}

void
MainWindow::processLiveFF(const size_t aDeviceIndex,
                          const size_t aExercise,
                          const size_t /* aIndex */,
                          const double aFitFactor)
{
  mUI->liveFFGraph->addDatapoint(aDeviceIndex, aExercise, aFitFactor);
}

void
MainWindow::subjectOrSpecimenEntryChanged(const QString&)
{
  bool enableTestStartPanel =
    mUI->subjectSelector->currentText().length() > 0 &&
    mUI->specimenSelector->currentText().length() > 0;
  mUI->startTestGroupBox->setEnabled(enableTestStartPanel);
}

void
MainWindow::refreshStatusBar()
{
  QString message = QString("Incolata v") + INCOLATA_VERSION;

  if (mDevice != nullptr) {
    P8020DeviceProperties* deviceProperties =
      p8020_device_get_properties(mDevice);
    if (deviceProperties != nullptr) {
      message = QString("%0    8020(A): #%2 (Last service: %3-%4, runtime "
                        "since last service: %5 h.)")
                  .arg(message,
                       deviceProperties->serial_number,
                       QString::number(deviceProperties->last_service_year),
                       QString::number(deviceProperties->last_service_month),
                       QString::number(size_t(
                         deviceProperties->run_time_since_last_service_hours)));
      p8020_device_properties_free(deviceProperties);
    }
  }

  statusBar()->showMessage(message);
}

void
MainWindow::testCompleted()
{
  mUI->testControlGroupBox->setEnabled(true);
}

MainWindow::MainWindow(const QString& aDevice, QWidget* const parent)
  : QMainWindow(parent)
  , mUI(new Ui::MainWindow)
  , mModel(new ExercisesModel)
  , mProtocolsModel(new ProtocolsModel)
  , mDevice(nullptr)
  , mWorkerThread(new QThread)
{
  mUI->setupUi(this);

  // There doesn't appear to be away to set this in xml layout.
  mUI->mainAreaSplitter->setSizes({400, 1000});

  mUI->protocolSelector->setModel(mProtocolsModel.get());

  // https://github.com/ahunt/incolata/issues/14 - Qt makes the dropdown too
  // narrow, the workaround is to set a min width. 800 was picked arbitrarily,
  // a dynamic calculation might be better but this works well enough for me.
  mUI->protocolSelector->view()->setMinimumWidth(800);

  mUI->exercisesList->setItemDelegate(new ExerciseRowDelegate(this));
  mUI->exercisesList->setModel(mModel.get());
  mModel->setCurrentExerciseLabel(mUI->currentExerciseLabel);

  mUI->rawChartView->setTitle("Raw Particle Conc. (#/cm³)");
  mUI->rawChartView->setXRange(sRawSampleRange);
  mUI->rawChartView->enableFixedXAxis();
  mUI->rawChartView->xAxis()->setTickInterval(30);
  mUI->rawChartView->yAxisLeft()->setRange(0, 2000);
  mUI->rawChartView->yAxisRight()->setRange(0, 20);

  mUI->ambientSampleGraph->setTitle("Ambient Particle Conc. (#/cm³)");
  mUI->ambientSampleGraph->setXRange(sAmbientSampleRange);
  mUI->ambientSampleGraph->setYAxisScalingMode(YAxisScalingMode::Floating);
  mUI->ambientSampleGraph->yAxisLeft()->setRange(1000, 10000);
  mUI->ambientSampleGraph->yAxisRight()->setRange(1000, 10000);

  mUI->specimenSampleGraph->setTitle("Specimen Particle Conc. (#/cm³)");
  mUI->specimenSampleGraph->setXRange(sSpecimenSampleRange);
  mUI->specimenSampleGraph->yAxisLeft()->setRange(0, 20);
  mUI->specimenSampleGraph->yAxisRight()->setRange(0, 2);

  mUI->liveFFGraph->setTitle("Live Fit Factor");
  mUI->liveFFGraph->enableLogYAxis();
  // Intentionally use the same range as for specimen samples - the two graphs
  // should mirror each other:
  mUI->liveFFGraph->setXRange(sSpecimenSampleRange);

  mUI->specimenIconLabel->setPixmap(
    QIcon(":/specimen.svg").pixmap(QSize(32, 32)));
  mUI->subjectIconLabel->setPixmap(
    QIcon(":/subject.svg").pixmap(QSize(32, 32)));

  // The QComboBox.placeHolder is ignored for editable QComboBoxes - the docs
  // explicitly state that you need to this instead...
  mUI->subjectSelector->lineEdit()->setPlaceholderText("Subject");
  mUI->specimenSelector->lineEdit()->setPlaceholderText("Specimen");

  refreshStatusBar();

  QObject::connect(mUI->startTest1,
                   &QAbstractButton::pressed,
                   this,
                   &MainWindow::startTestPressed);
  QObject::connect(mUI->startTest2,
                   &QAbstractButton::pressed,
                   this,
                   &MainWindow::startTestPressed);
  QObject::connect(mUI->startTest3,
                   &QAbstractButton::pressed,
                   this,
                   &MainWindow::startTestPressed);
  QObject::connect(mUI->startCustomProtocol,
                   &QAbstractButton::pressed,
                   this,
                   &MainWindow::startTestPressed);

  QObject::connect(this,
                   &MainWindow::exerciseChanged,
                   mModel.get(),
                   &ExercisesModel::updateCurrentExercise);
  QObject::connect(
    this, &MainWindow::ffUpdated, mModel.get(), &ExercisesModel::updateFF);
  QObject::connect(this,
                   &MainWindow::receivedInterimFF,
                   mModel.get(),
                   &ExercisesModel::renderInterimFF);
  QObject::connect(this,
                   &MainWindow::ffUpdated,
                   mUI->ffGraph,
                   &FitFactorChartView::addDatapoint);
  QObject::connect(this,
                   &MainWindow::renderRawSample,
                   mUI->rawCountLCD,
                   QOverload<const QString&>::of(&QLCDNumber::display));
  QObject::connect(mUI->specimenSelector,
                   &QComboBox::currentTextChanged,
                   this,
                   &MainWindow::subjectOrSpecimenEntryChanged);
  QObject::connect(mUI->subjectSelector,
                   &QComboBox::currentTextChanged,
                   this,
                   &MainWindow::subjectOrSpecimenEntryChanged);
  // Use signals+slots here as it deals with cross-thread dispatch for us.
  QObject::connect(
    this, &MainWindow::receivedRawSample, this, &MainWindow::processRawSample);
  QObject::connect(
    this, &MainWindow::receivedSample, this, &MainWindow::processSample);
  QObject::connect(
    this, &MainWindow::receivedLiveFF, this, &MainWindow::processLiveFF);

  const std::string deviceStdString = aDevice.toStdString();
  assert(deviceStdString.find_first_of('\0') == std::string::npos &&
         "device name must not contain nulls");
  mDevice =
    p8020_device_connect(deviceStdString.c_str(), &device_callback, this);

  TestWorker* testWorker = new TestWorker(mDevice);
  testWorker->moveToThread(mWorkerThread);
  connect(this, &MainWindow::triggerTest, testWorker, &TestWorker::runTest);
  connect(
    testWorker, &TestWorker::testCompleted, this, &MainWindow::testCompleted);
  mWorkerThread->start();
}

void
MainWindow::startTest(QSharedPointer<Protocol> protocol)
{
  mUI->testControlGroupBox->setEnabled(false);

  // TODO: move most of this into Protocol.
  const TestConfig* config;
  switch (protocol->tag) {
    case Protocol::BUILTIN_CONFIG:
      config = protocol->builtinConfig;
      break;
    case Protocol::BUILTIN_CONFIG_ID:
      const std::string shortNameStdString =
        protocol->builtinConfigID->toStdString();
      assert(shortNameStdString.find_first_of('\0') == std::string::npos &&
             "short name must not contain nulls");
      config = p8020_test_config_builtin_get(shortNameStdString.c_str());
      protocol.reset(new Protocol{
        .tag = Protocol::BUILTIN_CONFIG,
        .builtinConfig = config,
      });
      break;
  }

  const size_t exerciseCount = p8020_test_config_exercise_count(config);
  QStringList exercises;
  for (size_t i = 0; i < exerciseCount; i++) {
    auto name = p8020_test_config_exercise_name(config, i);
    exercises << name;
    p8020_string_free(name);
  }

  mUI->ffGraph->wipeData();
  mUI->ffGraph->setExerciseCount(exerciseCount);
  mModel->setExercises(exercises);

  mUI->ambientSampleGraph->wipeData();
  mUI->specimenSampleGraph->wipeData();
  mUI->liveFFGraph->wipeData();

  qInfo() << "Starting test: " << protocol->id() << " (" << exercises.length()
          << " exercises)";

  emit triggerTest(protocol,
                   &test_callback,
                   this,
                   mUI->specimenSelector->currentText(),
                   mUI->subjectSelector->currentText(),
                   mUI->commentEntry->text());
}

void
MainWindow::startTestPressed()
{
  auto sndr(sender());
  if (sndr == mUI->startTest1) {
    startTest(QSharedPointer<Protocol>(new Protocol{
      .tag = Protocol::BUILTIN_CONFIG_ID,
      .builtinConfigID = new QString("osha_legacy"),
    }));
  } else if (sndr == mUI->startTest2) {
    startTest(QSharedPointer<Protocol>(new Protocol{
      .tag = Protocol::BUILTIN_CONFIG_ID,
      .builtinConfigID = new QString("crash2.5"),
    }));
  } else if (sndr == mUI->startTest3) {
    startTest(QSharedPointer<Protocol>(new Protocol{
      .tag = Protocol::BUILTIN_CONFIG_ID,
      .builtinConfigID = new QString("live_mode_1h"),
    }));
  } else if (sndr == mUI->startCustomProtocol) {
    startTest(mProtocolsModel->protocol(mUI->protocolSelector->currentIndex()));
  } else {
    qWarning() << "Bad sender for startExercisesPressed()";
  }
}

MainWindow::~MainWindow() {
  // There is no special reason to connect these here... but there's no good
  // reason to connect them earlier anyway (we may have crashed in the meantime
  // ¯\_(ツ)_/¯.
  connect(mWorkerThread, &QThread::finished, mWorkerThread, &QThread::deleteLater);
  // TODO: make sure any ongoing work (in the worker) is interrupted too.
  // That's not possible yet because tests are still synchronous.
  mWorkerThread->quit();
}
