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
        default:
          // TODO: handle all cases.
          break;
      }
      break;
    }
  }
}

void
MainWindow::processRawSample(double sample)
{
  rawSeries->append(QPoint(rawSeries->count(), sample));
  // TODO: check range dynamically and/or extract constants.
  // TODO: check Y range.
  if (rawSeries->count() > sRawSampleRange) {
    // TODO: store a ref to the axis to avoid the lookup.
    rawChart->axes(Qt::Horizontal)[0]->setRange(rawSeries->count() - sRawSampleRange,
                                                rawSeries->count());
  }
}

void
MainWindow::processSample(SampleType sampleType,
                          size_t exercise,
                          size_t index,
                          double value)
{
  (void)index;
  switch (sampleType) {
    case SampleType::ambientSample: {
      QLineSeries* series;
      qreal x;
      if (mAmbientSampleSeriess.size() <= exercise) {
        if (mAmbientSampleSeriess.size() == 0) {
          x = 0;
        } else {
          // Overlap is deliberate.
          x = mAmbientSampleSeriess.back()->points().last().x();
        }

        series =
          mAmbientSampleSeriess.emplace_back(std::make_unique<QLineSeries>())
            .get();
        series->setPointsVisible(true);
        // Ordering is significant: Qt complains (and ignores the request) if
        // you try to attach an axis to a new series, if that axis is already
        // attached to another series which is attached to a chart, and if that
        // new series is not yet attached to the chart. (Translation:
        // addSeries(foo), before you foo->attachAxis.)
        mAmbientSampleChart->addSeries(series);
        series->attachAxis(mAmbientSampleXAxis.get());
        series->attachAxis(mAmbientSampleYAxis.get());
      } else {
        series = mAmbientSampleSeriess.back().get();
        x = series->points().last().x() + 1;
      }
      series->append(QPoint(x, value));

      // TODO: figure out a better way of doing this. It doesn't need to be
      // full-on functional, but surely there's something a bit less ugly.
      mAmbientMaxSeen = std::max(mAmbientMaxSeen, value);
      mAmbientMinSeen = std::min(mAmbientMinSeen, value);
      mAmbientSampleYAxis->setRange(mAmbientMinSeen - 100,
                                    mAmbientMaxSeen + 100);

      if (x > sAmbientSampleRange) {
        mAmbientSampleXAxis->setRange(x - sAmbientSampleRange, x);
      }
      break;
    }
    default:
      // TODO: handle remaining cases.
      break;
  }
}

void
MainWindow::receivedFF(uint ex, double ff)
{
  // TODO: validate that ordering is correct. It should be, but you never
  // know...
  ffSeries->append(QPoint(ex + 1, ff));
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , model(new ExercisesModel)
  , rawChart(new QChart)
  , rawSeries(new QLineSeries)
  , ffChart(new QChart)
  , ffSeries(new QLineSeries)
  , mFFXAxis(new QValueAxis)
  , mAmbientSampleChart(new QChart)
  , mAmbientSampleSeriess()
  , mAmbientSampleXAxis(new QValueAxis)
  , mAmbientSampleYAxis(new QValueAxis)
  , mAmbientMaxSeen(0.0)
  , mAmbientMinSeen(std::numeric_limits<double>::infinity())
  , workerThread(new QThread)
{
  ui->setupUi(this);

  ui->testTable->setModel(model);
  ui->testTable->setHorizontalHeader(nullptr);
  ui->testTable->setColumnWidth(1, 400);
  ui->testTable->setColumnWidth(2, 120);

  // TODO: move all this into a standalone class?
  rawChart->setAnimationOptions(QChart::SeriesAnimations);
  rawChart->setTitle("Raw Particle Count");
  rawChart->legend()->hide();
  rawChart->addSeries(rawSeries.get());
  auto xAxis = new QValueAxis();
  xAxis->setRange(0, sRawSampleRange);
  xAxis->setLabelFormat("%d");
  rawChart->addAxis(xAxis, Qt::AlignBottom);
  rawSeries->attachAxis(xAxis);
  auto yAxis = new QValueAxis();
  yAxis->setRange(0, 2500);
  yAxis->setTickCount(6);
  yAxis->setLabelFormat("%d");
  rawChart->addAxis(yAxis, Qt::AlignLeft);
  rawSeries->attachAxis(yAxis);
  ui->rawChartView->setChart(rawChart.get());

  // TODO; try to show a log axis on the right?
  ffChart->setAnimationOptions(QChart::SeriesAnimations);
  ffChart->setTitle("Fit Factors");
  ffChart->legend()->hide();
  ffChart->addSeries(ffSeries.get());
  mFFXAxis->setRange(0, 9);
  // TODO: figure out how to omit the first and last tick.
  // setTickInterval() does not work as documented, so we use tick count
  // instead.
  mFFXAxis->setTickCount(10);
  mFFXAxis->setLabelFormat("%d");
  QFont ffxAxisFont;
  ffxAxisFont.setPixelSize(10);
  mFFXAxis->setLabelsFont(ffxAxisFont);
  ffChart->addAxis(mFFXAxis.get(), Qt::AlignBottom);
  ffSeries->attachAxis(mFFXAxis.get());
  auto ffyAxis = new QLogValueAxis();
  ffyAxis->setLabelFormat("%d");
  ffyAxis->setMinorTickCount(-1);
  ffyAxis->setMax(1000);
  ffChart->addAxis(ffyAxis, Qt::AlignLeft);
  ffSeries->attachAxis(ffyAxis);
  ffSeries->setPointsVisible(true);
  ui->ffChartView->setChart(ffChart.get());
  ffyAxis->setRange(0, 1000);
  ffyAxis->setBase(10.0);

  mAmbientSampleChart->setAnimationOptions(QChart::SeriesAnimations);
  mAmbientSampleChart->setTitle("Ambient samples");
  mAmbientSampleChart->legend()->hide();
  mAmbientSampleXAxis->setRange(0, sAmbientSampleRange);
  mAmbientSampleXAxis->setLabelFormat("%d");
  mAmbientSampleChart->addAxis(mAmbientSampleXAxis.get(), Qt::AlignBottom);
  mAmbientSampleYAxis->setRange(1000, 10000);
  mAmbientSampleChart->addAxis(mAmbientSampleYAxis.get(), Qt::AlignLeft);
  ui->ambientSampleGraph->setChart(mAmbientSampleChart.get());

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
  QObject::connect(this, &MainWindow::ffUpdated, this, &MainWindow::receivedFF);
  QObject::connect(this,
                   &MainWindow::renderRawSample,
                   ui->rawCountLCD,
                   QOverload<const QString&>::of(&QLCDNumber::display));
  // Use signals+slots here as it deals with cross-thread dispatch for us.
  QObject::connect(
    this, &MainWindow::receivedRawSample, this, &MainWindow::processRawSample);
  QObject::connect(
    this, &MainWindow::receivedSample, this, &MainWindow::processSample);

  // TODO: provide a proper connection UI.
  device = device_connect("/dev/ttyUSB1");

  // TODO: connect remaining signals, e.g. thread (test) finish -> stuff in
  // the UI.
  TestWorker* testWorker = new TestWorker(device);
  testWorker->moveToThread(workerThread.get());
  connect(this, &MainWindow::triggerTest, testWorker, &TestWorker::runTest);
  workerThread->start();
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

  mFFXAxis->setRange(0, exercises.length() + 1);
  mFFXAxis->setTickCount(exercises.length() + 2);

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

MainWindow::~MainWindow() {}
