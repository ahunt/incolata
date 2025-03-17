#ifndef EXERCISEROWWIDGET_H
#define EXERCISEROWWIDGET_H

#include <QWidget>

namespace Ui {
class ExerciseRowWidget;
}

class ExerciseRowWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ExerciseRowWidget(QWidget* aParent = nullptr);
  ~ExerciseRowWidget();

private:
  std::unique_ptr<Ui::ExerciseRowWidget> mUI;
};

#endif // EXERCISEROWWIDGET_H
