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
      const uint ex(notification->exercise_result._0);
      const double ff(notification->exercise_result._1);
      emit mw->ffUpdated(ex, ff);
      break;
    };
    case TestNotification::Tag::RawSample: {
      const double sample = notification->raw_sample._0;
      emit mw->receivedRawSample(sample);
      if (sample < 100) {
        emit mw->renderRawSample(QString::number(sample, 'f', 2));
      } else {
        emit mw->renderRawSample(QString::number(sample, 'f', 0));
      }
      break;
    }
    case TestNotification::Tag::Sample: {
      auto& sample = notification->sample._0;
      switch (sample.tag) {
        case Sample::Tag::AmbientSample:
          emit mw->receivedSample(SampleType::ambientSample,
                                  sample.ambient_sample.exercise,
                                  sample.ambient_sample.index,
                                  sample.ambient_sample.value);
          break;
        case Sample::Tag::SpecimenSample:
          emit mw->receivedSample(SampleType::specimenSample,
                                  sample.specimen_sample.exercise,
                                  sample.specimen_sample.index,
                                  sample.specimen_sample.value);
          break;
        default:
          // TODO: handle all cases.
          break;
      }
      break;
    }
    case TestNotification::Tag::LiveFF: {
      auto& ff = notification->live_ff;
      emit mw->receivedLiveFF(ff.exercise, ff.index, ff.fit_factor);
      break;
    }
  }
}

void
MainWindow::processRawSample(double sample)
{
  ui->rawChartView->addDatapoint(0, sample);
}

void
MainWindow::processSample(SampleType sampleType,
                          size_t exercise,
                          size_t index,
                          double value)
{
  (void)index;
  switch (sampleType) {
    case SampleType::ambientSample:
      ui->ambientSampleGraph->addDatapoint(exercise, value);
      break;
    case SampleType::specimenSample:
      ui->specimenSampleGraph->addDatapoint(exercise, value);
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
  ui->liveFFGraph->addDatapoint(exercise, fit_factor);
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , model(new ExercisesModel)
  , mWorkerThread(new QThread)
{
  ui->setupUi(this);

  ui->testTable->setModel(model);
  ui->testTable->setHorizontalHeader(nullptr);
  ui->testTable->setColumnWidth(0, 20);
  ui->testTable->setColumnWidth(1, 400);
  ui->testTable->setColumnWidth(2, 120);

  ui->rawChartView->setTitle("Raw samples");
  ui->rawChartView->setXRange(sRawSampleRange);
  ui->rawChartView->xAxis()->setTickInterval(30);
  ui->rawChartView->yAxis()->setRange(0, 2000);
  ui->rawChartView->yAxis()->setRange(0, 2000);

  ui->ambientSampleGraph->setTitle("Ambient Samples");
  ui->ambientSampleGraph->setXRange(sAmbientSampleRange);
  ui->ambientSampleGraph->setYAxisScalingMode(YAxisScalingMode::Floating);
  ui->ambientSampleGraph->yAxis()->setRange(1000, 10000);

  ui->specimenSampleGraph->setTitle("Specimen Samples");
  ui->specimenSampleGraph->setXRange(sSpecimenSampleRange);
  ui->specimenSampleGraph->yAxis()->setRange(0, 20);

  ui->liveFFGraph->setTitle("Live(ish) Fit Factor");
  ui->liveFFGraph->enableLogYAxis();
  // Intentionally use the same range as for specimen samples - the two graphs
  // should mirror each other:
  ui->liveFFGraph->setXRange(sSpecimenSampleRange);

  QObject::connect(ui->startTest1,
                   &QAbstractButton::pressed,
                   this,
                   &MainWindow::startTestPressed);
  QObject::connect(ui->startTest2,
                   &QAbstractButton::pressed,
                   this,
                   &MainWindow::startTestPressed);
  // TODO: implement custom exercise support.

  QObject::connect(this,
                   &MainWindow::exerciseChanged,
                   model,
                   &ExercisesModel::updateCurrentExercise);
  QObject::connect(
    this, &MainWindow::ffUpdated, model, &ExercisesModel::updateFF);
  QObject::connect(this,
                   &MainWindow::ffUpdated,
                   ui->ffGraph,
                   &FitFactorChartView::addDatapoint);
  QObject::connect(this,
                   &MainWindow::renderRawSample,
                   ui->rawCountLCD,
                   QOverload<const QString&>::of(&QLCDNumber::display));
  // Use signals+slots here as it deals with cross-thread dispatch for us.
  QObject::connect(
    this, &MainWindow::receivedRawSample, this, &MainWindow::processRawSample);
  QObject::connect(
    this, &MainWindow::receivedSample, this, &MainWindow::processSample);
  QObject::connect(
    this, &MainWindow::receivedLiveFF, this, &MainWindow::processLiveFF);

  // TODO: provide a proper connection UI.
  device = device_connect("/dev/ttyUSB1");

  // TODO: connect remaining signals, e.g. worker (test) finish -> stuff in
  // the UI.
  TestWorker* testWorker = new TestWorker(device);
  testWorker->moveToThread(mWorkerThread);
  connect(this, &MainWindow::triggerTest, testWorker, &TestWorker::runTest);
  mWorkerThread->start();
}

void
MainWindow::startTest(const QStringList& exercises,
                      const QString& protocolShortName)
{
  qInfo() << "Start test: " << exercises.length() << " exercises";

  // TODO: reenable these after the test.
  ui->startTest1->setEnabled(false);
  ui->startTest2->setEnabled(false);
  ui->specimenSelector->setEnabled(false);
  ui->subjectSelector->setEnabled(false);

  model->setExercises(exercises);
  ui->ffGraph->setExerciseCount(exercises.count());

  TestConfig* config = test_config_new(exercises.length());
  config->test_callback = &test_callback;
  config->test_callback_data = this;

  emit triggerTest(config,
                   ui->specimenSelector->currentText(),
                   ui->subjectSelector->currentText(),
                   protocolShortName);

  // TODO: reset graphs if necessary.
}

void
MainWindow::startTestPressed()
{
  auto sndr(sender());
  if (sndr == ui->startTest1) {
    startTest(QStringList() << "Normal breathing"
                            << "Deep Breathing"
                            << "Turning head side to side"
                            << "Moving head up and down"
                            << "Talking"
                            << "Grimace"
                            << "Bending over"
                            << "Normal breathing",
              "OSHA");
  } else if (sndr == ui->startTest2) {
    startTest(QStringList() << "Relax"
                            << "2x Jaw Motion cycles"
                            << "Relax"
                            << "Relax"
                            << "2x Jaw Motion cycles"
                            << "Relax"
                            << "Relax"
                            << "2x Jaw Motion cycles"
                            << "Relax"
                            << "Relax"
                            << "2x Jaw Motion cycles"
                            << "Relax",
              "FTTP2.5");
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
