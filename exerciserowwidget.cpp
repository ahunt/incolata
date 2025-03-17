#include "exerciserowwidget.h"
#include "ui_exerciserowwidget.h"

ExerciseRowWidget::ExerciseRowWidget(QWidget* aParent)
  : QWidget(aParent)
  , mUI(new Ui::ExerciseRowWidget)
{
  mUI->setupUi(this);
}

ExerciseRowWidget::~ExerciseRowWidget() {}
