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
  void processLiveFF(size_t exercise, size_t index, double fit_factor);

private:
  std::unique_ptr<Ui::MainWindow> mUI;
  ExercisesModel* mModel;
  Device* mDevice;

  // Owned by itself, self-destructs as and when needed.
  QThread *mWorkerThread;

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
  void ffUpdated(const uint& exercise, const double& fitFactor);
  void renderRawSample(QString sample);
  void receivedRawSample(double sample);
  void receivedSample(SampleType sampleType,
                      size_t exercise,
                      size_t index,
                      double value);
  void receivedLiveFF(size_t exercise, size_t index, double fit_factor);
};
#endif // MAINWINDOW_H
