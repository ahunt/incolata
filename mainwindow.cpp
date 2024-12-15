#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QChart>
#include <QLineSeries>
#include <QLogValueAxis>
#include <QThread>
#include <QValueAxis>

#include "exercisesmodel.h"
#include "libp8020/libp8020.h"
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
      emit mw->receivedRawSample(sample);
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
      const size_t ex = notification->exercise_result._0;
      const double ff = notification->exercise_result._1;
      const double err = notification->exercise_result._2;
      emit mw->ffUpdated(ex, ff, err);
      break;
    };
    case TestNotification::Tag::Sample: {
      auto& sample = notification->sample._0;
      switch (sample.sample_type) {
        case SampleType::AmbientSample:
        case SampleType::SpecimenSample:
          emit mw->receivedSample(
            sample.sample_type, sample.exercise, sample.value);
          break;
        case SampleType::AmbientPurge:
        case SampleType::SpecimenPurge:
          break;
      }
      break;
    }
    case TestNotification::Tag::LiveFF: {
      auto& ff = notification->live_ff;
      emit mw->receivedLiveFF(ff.exercise, ff.index, ff.fit_factor);
      break;
    }
    case TestNotification::Tag::InterimFF:
      auto& ff = notification->interim_ff;
      emit mw->receivedInterimFF(ff.exercise, ff.fit_factor);
      break;
  }
}

void
MainWindow::processRawSample(double sample)
{
  mUI->rawChartView->addDatapoint(0, sample);
}

void
MainWindow::processSample(SampleType sampleType, size_t exercise, double value)
{
  switch (sampleType) {
    case SampleType::AmbientSample:
      mUI->ambientSampleGraph->addDatapoint(exercise, value);
      break;
    case SampleType::SpecimenSample:
      mUI->specimenSampleGraph->addDatapoint(exercise, value);
      break;
    default:
      // TODO: handle remaining cases.
      break;
  }
}

void
MainWindow::processLiveFF(size_t exercise, size_t index, double fit_factor)
{
  (void)index;
  mUI->liveFFGraph->addDatapoint(exercise, fit_factor);
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , mUI(new Ui::MainWindow)
  , mModel(new ExercisesModel)
  , mWorkerThread(new QThread)
{
  mUI->setupUi(this);

  mUI->testTable->setModel(mModel.get());
  mUI->testTable->setHorizontalHeader(nullptr);
  mUI->testTable->setColumnWidth(0, 20);
  mUI->testTable->setColumnWidth(1, 400);
  mUI->testTable->setColumnWidth(2, 240);

  mUI->rawChartView->setTitle("Raw Particles");
  mUI->rawChartView->setXRange(sRawSampleRange);
  mUI->rawChartView->enableFixedXAxis();
  mUI->rawChartView->xAxis()->setTickInterval(30);
  mUI->rawChartView->yAxis()->setRange(0, 2000);

  mUI->ambientSampleGraph->setTitle("Ambient Particles");
  mUI->ambientSampleGraph->setXRange(sAmbientSampleRange);
  mUI->ambientSampleGraph->setYAxisScalingMode(YAxisScalingMode::Floating);
  mUI->ambientSampleGraph->yAxis()->setRange(1000, 10000);

  mUI->specimenSampleGraph->setTitle("Specimen Particles");
  mUI->specimenSampleGraph->setXRange(sSpecimenSampleRange);
  mUI->specimenSampleGraph->yAxis()->setRange(0, 20);

  mUI->liveFFGraph->setTitle("Live(ish) Fit Factor");
  mUI->liveFFGraph->enableLogYAxis();
  // Intentionally use the same range as for specimen samples - the two graphs
  // should mirror each other:
  mUI->liveFFGraph->setXRange(sSpecimenSampleRange);

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
  // Use signals+slots here as it deals with cross-thread dispatch for us.
  QObject::connect(
    this, &MainWindow::receivedRawSample, this, &MainWindow::processRawSample);
  QObject::connect(
    this, &MainWindow::receivedSample, this, &MainWindow::processSample);
  QObject::connect(
    this, &MainWindow::receivedLiveFF, this, &MainWindow::processLiveFF);

  // TODO: provide a proper connection UI.
  mDevice = p8020_device_connect("/dev/ttyUSB0", &device_callback, this);

  // TODO: connect remaining signals, e.g. worker (test) finish -> stuff in
  // the UI.
  TestWorker* testWorker = new TestWorker(mDevice);
  testWorker->moveToThread(mWorkerThread);
  connect(this, &MainWindow::triggerTest, testWorker, &TestWorker::runTest);
  mWorkerThread->start();
}

void
MainWindow::startTest(const QString& protocolShortName)
{
  // TODO: reenable these after the test.
  mUI->startTest1->setEnabled(false);
  mUI->startTest2->setEnabled(false);
  mUI->startTest3->setEnabled(false);
  mUI->specimenSelector->setEnabled(false);
  mUI->subjectSelector->setEnabled(false);

  const std::string shortNameStdString = protocolShortName.toStdString();
  assert(shortNameStdString.find_first_of('\0') == std::string::npos && "short name must not contain nulls");
  TestConfig* config =
    p8020_test_config_builtin_load(shortNameStdString.c_str());
  const size_t exerciseCount = p8020_test_config_exercise_count(config);
  QStringList exercises;
  for (size_t i = 0; i < exerciseCount; i++) {
    auto name = p8020_test_config_exercise_name(config, i);
    exercises << name;
    p8020_test_config_exercise_name_free(name);
  }

  mUI->ffGraph->setExerciseCount(exerciseCount);
  mModel->setExercises(exercises);

  qInfo() << "Start test: " << protocolShortName << " (" << exercises.length() << " exercises)";

  emit triggerTest(config,
                   (void*)&test_callback,
                   this,
                   mUI->specimenSelector->currentText(),
                   mUI->subjectSelector->currentText(),
                   protocolShortName);

  // TODO: reset graphs if necessary.
}

void
MainWindow::startTestPressed()
{
  auto sndr(sender());
  if (sndr == mUI->startTest1) {
    startTest("osha_legacy");
  } else if (sndr == mUI->startTest2) {
    startTest("crash2.5");
  } else if (sndr == mUI->startTest3) {
    startTest("osha_fast_ffp");
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
