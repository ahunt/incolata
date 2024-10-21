#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "exercisesmodel.h"
#include "p8020a-rs/libp8020a.h"
#include "testworker.h"

void
MainWindow::test_callback(const TestNotification* notification, void* cb_data)
{
  MainWindow* mw = static_cast<MainWindow*>(cb_data);
  (void)mw;
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
      break;
    }
    default:
      // TODO: handle all cases.
      break;
  }
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , model(new ExercisesModel)
  , workerThread(new QThread)
{
  ui->setupUi(this);

  ui->testTable->setModel(model);
  ui->testTable->setHorizontalHeader(nullptr);
  ui->testTable->setColumnWidth(1, 400);

  QObject::connect(ui->startTest1, &QAbstractButton::pressed,
                     this, &MainWindow::startTestPressed);
  QObject::connect(ui->startTest2, &QAbstractButton::pressed,
                     this, &MainWindow::startTestPressed);
  // TODO: implement custom exercise support.

  QObject::connect(this,
                   &MainWindow::exerciseChanged,
                   model,
                   &ExercisesModel::updateCurrentExercise);
  QObject::connect(
    this, &MainWindow::ffUpdated, model, &ExercisesModel::updateFF);
  // TODO: connect(this, &MainWindow::receivedRawSample,
  //               rawSamplesModel, &RawSampleModel::addSample);

  // TODO: provide a proper connection UI.
  device = device_connect("/dev/ttyUSB0");

  // TODO: connect remaining signals, e.g. thread (test) finish -> stuff in the
  // UI.
  TestWorker* testWorker = new TestWorker(device);
  testWorker->moveToThread(workerThread.get());
  connect(this, &MainWindow::triggerTest, testWorker, &TestWorker::runTest);
  workerThread->start();
}

void MainWindow::startTest(const QStringList& exercises) {
  qInfo() << "Start test: " << exercises.length() << " exercises";

  // TODO: reenable these after the test.
  ui->startTest1->setEnabled(false);
  ui->startTest2->setEnabled(false);
  ui->specimenSelector->setEnabled(false);
  ui->subjectSelector->setEnabled(false);

  model->setExercises(exercises);

  TestConfig* config = test_config_new(exercises.length());
  config->test_callback = &test_callback;
  config->test_callback_data = this;

  emit triggerTest(config);
}


void MainWindow::startTestPressed()
{
  auto sndr(sender());
  if (sndr == ui->startTest1) {
    startTest(QStringList() << "Normal breathing" << "Deep Breathing" << "Turning head side to side" << "Moving head up and down" << "Talking" << "Grimace" << "Bending over" << "Normal breathing");
    } else if     (sndr == ui->startTest2) {
    startTest(QStringList()
	      << "Relax" << "2x Jaw Motion cycles" << "Relax"
	      << "Relax" << "2x Jaw Motion cycles" << "Relax"
	      << "Relax" << "2x Jaw Motion cycles" << "Relax"
	      << "Relax" << "2x Jaw Motion cycles" << "Relax");
  } else {
    qWarning() << "Bad sender for startExercisesPressed()";
  }
}

MainWindow::~MainWindow() {}
