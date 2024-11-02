#ifndef FITFACTORCHARTVIEW_H
#define FITFACTORCHARTVIEW_H

#include <QChartView>

class QLineSeries;
class QLogValueAxis;
class QValueAxis;

class FitFactorChartView : public QChartView
{
public:
  FitFactorChartView(QWidget* const parent = nullptr);

  // The number of exercises that will be displayed.
  Q_PROPERTY(size_t exerciseCount MEMBER mExerciseCount WRITE setExerciseCount);
  void setExerciseCount(const qsizetype& count);

public slots:
  void addDatapoint(const size_t& exercise, const double& ff);

private:
  qsizetype mExerciseCount;
  qreal mMaxFFSeen;

  // Owned by QChartView.chart().
  QLineSeries* const mSeries;
  // Owned by QChartView.chart().
  QValueAxis* const mXAxis;
  // Owned by QChartView.chart().
  QLogValueAxis* const mYAxis;

  void refreshExerciseRange();
};

#endif // FITFACTORCHARTVIEW_H
