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
  Q_PROPERTY(size_t exerciseCount MEMBER mExerciseCount WRITE setExerciseCount)
  void setExerciseCount(const qsizetype& count);

  // Wipes all data shown on the graph. This is totally independent of
  // setExerciseCount(), although in general they are called as a pair.
  void wipeData();

public slots:
  void addDatapoint(const size_t& deviceIndex,
                    const size_t& exercise,
                    const double& ff);

private:
  qsizetype mExerciseCount;
  qreal mMaxFFSeen;

  // Members are owned by QChartView.chart().
  std::vector<QLineSeries*> const mSeriess;
  // Owned by QChartView.chart().
  QValueAxis* const mXAxis;
  // Owned by QChartView.chart().
  QLogValueAxis* const mYAxis;

  void refreshExerciseRange();
  void refreshBackground();
};

#endif // FITFACTORCHARTVIEW_H
