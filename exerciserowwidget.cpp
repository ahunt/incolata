#include "exerciserowwidget.h"
#include "ui_exerciserowwidget.h"

#include <algorithm>

#include <QPainter>

ExerciseRowWidget::ExerciseRowWidget(QWidget* aParent)
  : QWidget(aParent)
  , mUI(new Ui::ExerciseRowWidget)
{
  mUI->setupUi(this);
}

ExerciseRowWidget::~ExerciseRowWidget() {}

void
setFFLabels(const size_t& aIndex,
            QLabel* const aLogFFLabel,
            QLabel* const aLinFFLabel,
            const ExerciseData& aData)
{
  const bool hasData =
    aData.mFitFactors.size() > aIndex && aData.mFitFactors[aIndex].has_value();
  if (hasData) {
    // TODO: extract & centralise FF formatting.
    const auto& ff = aData.mFitFactors[aIndex].value();
    QString linFF =
      (ff >= 10) ? QString::number(ff, 'f', 0) : QString::number(ff, 'f', 1);
    if (aData.mFitFactorErrors.size() > 0 &&
        aData.mFitFactorErrors[0].has_value()) {
      linFF += "±";
      linFF += QString::number(aData.mFitFactorErrors[0].value(), 'f', 1);
    }
    aLinFFLabel->setText(linFF);

    aLogFFLabel->setText(QString::number(log10(ff), 'f', 2));

    if (aData.mIsInterim) {
      QString styleSheet = "color: palette(link-visited)";
      aLinFFLabel->setStyleSheet(styleSheet);
      aLogFFLabel->setStyleSheet(styleSheet);
    } else {
      aLinFFLabel->setStyleSheet("");
      aLogFFLabel->setStyleSheet("");
    }
  } else {
    // Set empty text, as opposed to changing visibility, to minimise
    // jarring relayouting.
    aLinFFLabel->setText("");
    aLogFFLabel->setText("");
  }
}

void
ExerciseRowWidget::setData(const ExerciseData& aData)
{
  mUI->label_exercise_title->setText(aData.mTitle);

  // some languages:
  // let hasAtLeastOneFF = data.fit_factors.iter().any(|&ff| ff.is_some());
  // "modern" C++:
  const bool hasAtLeastOneFF = std::ranges::any_of(
    aData.mFitFactors, [](std::optional<double> ff) { return ff.has_value(); });

  mUI->label_interim->setVisible(aData.mIsInterim && hasAtLeastOneFF);
  mUI->indicator->setVisible(aData.mIsRecording);

  // Yes, this could be better written to handle an arbitrary number of devices,
  // but would it be worth it?
  setFFLabels(0, mUI->label_dev0_logff, mUI->label_dev0_linff, aData);
  setFFLabels(1, mUI->label_dev1_logff, mUI->label_dev1_linff, aData);
}

ExerciseRowDelegate::ExerciseRowDelegate(QObject* aParent)
  : QStyledItemDelegate(aParent)
  , mWidget(new ExerciseRowWidget())
{
}

void
ExerciseRowDelegate::paint(QPainter* aPainter,
                           const QStyleOptionViewItem& aOption,
                           const QModelIndex& aIndex) const
{
  if (aIndex.data().canConvert<ExerciseData>()) {
    ExerciseData data = qvariant_cast<ExerciseData>(aIndex.data());
    mWidget->setData(data);

    // With thanks to nico88desmo for showing how this needs to be done
    // (https://forum.qt.io/post/539274).
    mWidget->resize(aOption.rect.size());
    aPainter->save();
    aPainter->translate(aOption.rect.topLeft());
    mWidget->render(aPainter);
    aPainter->restore();
  } else {
    QStyledItemDelegate::paint(aPainter, aOption, aIndex);
  }
}

QSize
ExerciseRowDelegate::sizeHint(const QStyleOptionViewItem& aOption,
                              const QModelIndex& aIndex) const
{
  if (aIndex.data().canConvert<ExerciseData>()) {
    ExerciseData data = qvariant_cast<ExerciseData>(aIndex.data());
    mWidget->setData(data);
    return mWidget->sizeHint();
  }
  return QStyledItemDelegate::sizeHint(aOption, aIndex);
}
