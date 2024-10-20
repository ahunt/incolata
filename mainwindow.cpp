#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "exercisesmodel.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , model(new ExercisesModel)
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
}

void MainWindow::startTest(const QStringList& exercises) {
  qInfo() << "Start test: " << exercises.length() << " exercises";

  // TODO: reenable these after the test.
  ui->startTest1->setEnabled(false);
  ui->startTest2->setEnabled(false);
  ui->specimenSelector->setEnabled(false);
  ui->subjectSelector->setEnabled(false);

  model->setExercises(exercises);
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
