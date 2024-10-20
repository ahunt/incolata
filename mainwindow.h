#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
  std::unique_ptr<Ui::MainWindow> ui;
  ExercisesModel* model;

  void startTest(const QStringList& exercises);
};
#endif // MAINWINDOW_H
