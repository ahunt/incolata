#ifndef EXERCISEROWWIDGET_H
#define EXERCISEROWWIDGET_H

#include <QStyledItemDelegate>
#include <QWidget>

namespace Ui {
class ExerciseRowWidget;
class ExerciseRowDelegate;
}

struct ExerciseData
{
  // title, including index if desired (E.g. "1. Foo bar").
  QString mTitle;
  // Per-device fit-factors.
  std::vector<std::optional<double>> mFitFactors;
  // Per-device errors.
  std::vector<std::optional<double>> mFitFactorErrors;
  bool mIsInterim;
  bool mIsRecording;
};

Q_DECLARE_METATYPE(ExerciseData);

class ExerciseRowWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ExerciseRowWidget(QWidget* aParent = nullptr);
  ~ExerciseRowWidget();
  void setData(const ExerciseData& aData);

private:
  std::unique_ptr<Ui::ExerciseRowWidget> mUI;
};

class ExerciseRowDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  ExerciseRowDelegate(QObject* aParent = nullptr);

  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

private:
  std::unique_ptr<ExerciseRowWidget> mWidget;
};

#endif // EXERCISEROWWIDGET_H
