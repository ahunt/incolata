#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "incolata_common.h"

class QChart;
class QLineSeries;
class QLogValueAxis;
class QThread;
class QValueAxis;

enum class SampleType;
struct TestNotification;
struct P8020Device;
struct P8020DeviceNotification;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ExercisesModel;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(const QString& aDevice, QWidget* const parent = nullptr);
  ~MainWindow();

private slots:
  void startTestPressed();
  void processRawSample(double sample);
  void processSample(SampleType sampleType, size_t exercise, double value);
  void processLiveFF(size_t exercise, size_t index, double fit_factor);
  void subjectOrSpecimenEntryChanged(const QString& aContents);

private:
  std::unique_ptr<Ui::MainWindow> mUI;
  std::unique_ptr<ExercisesModel> mModel;
  P8020Device* mDevice;

  // Owned by itself, self-destructs as and when needed.
  QThread *mWorkerThread;

  static void device_callback(const P8020DeviceNotification* notification,
                              void* cb_data);
  static void test_callback(const TestNotification* notification,
                            void* cb_data);
  void startTest(const QString& protocolShortName);

signals:
  void triggerTest(TestConfig* const config,
                   void* const callback,
                   void* const cb_data,
                   const QString& specimen,
                   const QString& subject,
                   const QString& protocol);
  void exerciseChanged(uint ex);
  void ffUpdated(const uint& exercise, const double& fitFactor, const double& err);
  void renderRawSample(QString sample);
  void receivedRawSample(double sample);
  void receivedSample(SampleType sampleType, size_t exercise, double value);
  void receivedLiveFF(size_t exercise, size_t index, double fit_factor);
  void receivedInterimFF(size_t exercise, double fit_factor);
};
#endif // MAINWINDOW_H
