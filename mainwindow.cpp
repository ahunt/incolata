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
}

MainWindow::~MainWindow() {}
