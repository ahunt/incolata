#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QChart;
class QLineSeries;
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

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void startTestPressed();
  void processRawSample(double sample);
  void receivedFF(uint ex, double ff);

private:
  std::unique_ptr<Ui::MainWindow> ui;
  ExercisesModel* model;
  std::unique_ptr<QChart> rawChart;
  std::unique_ptr<QLineSeries> rawSeries;
  std::unique_ptr<QChart> ffChart;
  std::unique_ptr<QLineSeries> ffSeries;
  std::unique_ptr<QValueAxis> mFFXAxis;

  std::unique_ptr<QThread> workerThread;
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
};
#endif // MAINWINDOW_H
