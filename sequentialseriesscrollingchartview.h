#ifndef SEQUENTIALSERIESSCROLLINGCHARTVIEW_H
#define SEQUENTIALSERIESSCROLLINGCHARTVIEW_H

#include <QChartView>

class QAbstractAxis;
class QValueAxis;
class QLineSeries;

enum YAxisScalingMode
{
  ZeroAnchored,
  Floating,
};

// SequentialSeriesScrollingChartView allows you to easily visualise render
// sequential dataseries. It's raison d'être is the live sample/ambient/FF
// graphs, where each series is discrete, but should follow directly from the
// previous series. It can also be used as a single-series scrolling chart,
// by only ever to appending to the series with index 0.
class SequentialSeriesScrollingChartView : public QChartView
{
public:
  SequentialSeriesScrollingChartView(QWidget* const parent = nullptr);

  // The range of data to show on the X-Axis. The actual range is calculated
  // dynamically as [max(x) - xRange, max(x)].
  Q_PROPERTY(qsizetype xRange MEMBER mXRange WRITE setXRange)
  void setXRange(const qsizetype& range);

  Q_PROPERTY(QValueAxis* xAxis MEMBER mXAxis READ default)
  QValueAxis* xAxis() const;

  Q_PROPERTY(QAbstractAxis* yAxisLeft MEMBER mYAxisLeft READ default)
  QAbstractAxis* yAxisLeft() const;
  Q_PROPERTY(QAbstractAxis* yAxisRight MEMBER mYAxisRight READ default)
  QAbstractAxis* yAxisRight() const;

  // Spacing between each sequential series. All changes must be made prior to
  // adding any datapoints. Defaults to 0, i.e. by default the first point of
  // each series overlaps the last point of the previous series.
  Q_PROPERTY(size_t seriesSpacing MEMBER default WRITE setSeriesSpacing)
  void setSeriesSpacing(const size_t& spacing);

  // Set the Chart's title.
  void setTitle(const QString& title);
  // Enables a log-scale Y Axis. Must be called prior to adding any datapoints;
  // this action is irreversable.
  void enableLogYAxis();
  // Enables a fixed X Axis, from -xRange to 0 (and hides the actual X axis).
  // Must only be called once; this action is irreversable.
  void enableFixedXAxis();
  // Set the Y-Axis scaling mode. Must not be used if enableLogYAxis() is in
  // use (it's 1-anchored anyway).
  void setYAxisScalingMode(const YAxisScalingMode& mode);

  // It is not permitted to add a datapoint to a previous series.
  void addDatapoint(const size_t& deviceIndex,
                    const size_t& seriesIndex,
                    const qreal& value);

  // Wipe all chart data. After this call, it is safe to start adding data from
  // the series with index=0 again.
  void wipeData();

private:
  // Owned by QChartView (will be deleted by QChartView *unless* replaced via
  // QChartView::setChart()).
  QChart* const mChart;

  // Owned by mChart.
  QValueAxis* mXAxis;
  // Owned by mChart.
  QAbstractAxis* mYAxisLeft;
  // Owned by mChart.
  QAbstractAxis* mYAxisRight;
  // May be nullptr. Owned by mChart if it exists.
  QValueAxis* mFixedXAxis;

  // Each series is owned by mChart.
  // mSeriesLists[0] is device 0, left axis.
  // mSeriesLists[1] is device 1, right axis.
  std::vector<std::vector<QLineSeries*>> mSeriesLists;

  YAxisScalingMode mYAxisScalingMode;
  size_t mSeriesSpacing;
  qsizetype mXRange;

  qreal mMinYSeenLeft, mMaxYSeenLeft, mMinYSeenRight, mMaxYSeenRight;

  void refreshXRange();
};

#endif // SEQUENTIALSERIESSCROLLINGCHARTVIEW_H
