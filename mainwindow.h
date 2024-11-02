#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QChart;
class QLineSeries;
class QLogValueAxis;
class QThread;
class QValueAxis;

struct TestConfig;
struct TestNotification;
struct Device;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ExercisesModel;

// TODO: try to avoid duplicating the rust enum. We cannot (yet) send the rust
// enum via signals/slots.
enum SampleType
{
  ambientPurge,
  ambientSample,
  specimenPurge,
  specimenSample
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void startTestPressed();
  void processRawSample(double sample);
  void processSample(SampleType sampleType,
                     size_t exercise,
                     size_t index,
                     double value);
  void receivedFF(uint ex, double ff);
  void processLiveFF(size_t exercise, size_t index, double fit_factor);

private:
  std::unique_ptr<Ui::MainWindow> ui;
  ExercisesModel* model;

  std::unique_ptr<QChart> rawChart;
  std::unique_ptr<QLineSeries> rawSeries;

  std::unique_ptr<QChart> ffChart;
  std::unique_ptr<QLineSeries> ffSeries;
  std::unique_ptr<QValueAxis> mFFXAxis;

  std::unique_ptr<QChart> mSpecimenSampleChart;
  std::vector<std::unique_ptr<QLineSeries>> mSpecimenSampleSeriess;
  std::unique_ptr<QValueAxis> mSpecimenSampleXAxis;
  std::unique_ptr<QValueAxis> mSpecimenSampleYAxis;
  double mSpecimenMaxSeen;

  std::unique_ptr<QChart> mLiveFFChart;
  std::vector<std::unique_ptr<QLineSeries>> mLiveFFSeriess;
  std::unique_ptr<QValueAxis> mLiveFFXAxis;
  std::unique_ptr<QLogValueAxis> mLiveFFYAxis;

  QThread *mWorkerThread;
  Device* device;

  static void test_callback(const TestNotification* notification,
                            void* cb_data);
  void startTest(const QStringList& exercises,
                 const QString& protocolShortName);

signals:
  void triggerTest(TestConfig* testConfig,
                   const QString& specimen,
                   const QString& subject,
                   const QString& protocol);
  void exerciseChanged(uint ex);
  void ffUpdated(uint ex, double ff);
  void renderRawSample(QString sample);
  void receivedRawSample(double sample);
  void receivedSample(SampleType sampleType,
                      size_t exercise,
                      size_t index,
                      double value);
  void receivedLiveFF(size_t exercise, size_t index, double fit_factor);
};
#endif // MAINWINDOW_H
