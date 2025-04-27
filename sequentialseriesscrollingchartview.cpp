#include "sequentialseriesscrollingchartview.h"

#include <QLineSeries>
#include <QLogValueAxis>
#include <QValueAxis>

SequentialSeriesScrollingChartView::SequentialSeriesScrollingChartView(
  QWidget* const parent)
  : QChartView(parent)
  // QChartView already creates a chart for us during construction
  // (https://github.com/qt/qtcharts/blob/4d0f13651edd605eacf9748ed46185a7d4d47e21/src/charts/qchartview.cpp#L284-L285),
  // we only grab its pointer for convenience.
  , mChart(this->chart())
  , mXAxis(new QValueAxis())
  , mYAxisLeft(new QValueAxis())
  , mYAxisRight(new QValueAxis())
  , mFixedXAxis(nullptr)
  , mSeriesLists(2)
  , mYAxisScalingMode(YAxisScalingMode::ZeroAnchored)
  , mSeriesSpacing(0)
  , mMinYSeenLeft(std::numeric_limits<double>::infinity())
  , mMaxYSeenLeft(0.0)
  , mMinYSeenRight(std::numeric_limits<double>::infinity())
  , mMaxYSeenRight(0.0)
{
  mXAxis->setLabelFormat("%d");
  static_cast<QValueAxis*>(mYAxisLeft)->setLabelFormat("%d");
  static_cast<QValueAxis*>(mYAxisRight)->setLabelFormat("%d");

  mXAxis->setTickType(QValueAxis::TicksDynamic);
  mXAxis->setTickAnchor(0);
  mXAxis->setTickInterval(10);

  mChart->legend()->hide();
  mChart->addAxis(mXAxis, Qt::AlignBottom);
  mChart->addAxis(mYAxisLeft, Qt::AlignLeft);
  mChart->addAxis(mYAxisRight, Qt::AlignRight);
}

QValueAxis*
SequentialSeriesScrollingChartView::xAxis() const
{
  return mXAxis;
}

QAbstractAxis*
SequentialSeriesScrollingChartView::yAxisLeft() const
{
  return mYAxisLeft;
}

QAbstractAxis*
SequentialSeriesScrollingChartView::yAxisRight() const
{
  return mYAxisRight;
}

void
SequentialSeriesScrollingChartView::enableLogYAxis()
{
  assert(mSeriesLists[0].size() == 0 &&
         "enableLogYAxis() must only be called prior to adding datapoints");
  assert(mSeriesLists[1].size() == 0 &&
         "enableLogYAxis() must only be called prior to adding datapoints");

  mChart->removeAxis(mYAxisLeft);
  delete mYAxisLeft;
  // Use a temporary value with type QLogValueAxis as we're calling
  // QLogValueAxis-specific methods below.
  QLogValueAxis* yAxisLeft = new QLogValueAxis();
  mYAxisLeft = yAxisLeft;
  mChart->removeAxis(mYAxisRight);
  delete mYAxisRight;
  // See comment on yAxisLeft;
  QLogValueAxis* yAxisRight = new QLogValueAxis();
  mYAxisRight = yAxisRight;

  mChart->addAxis(yAxisLeft, Qt::AlignLeft);
  mChart->addAxis(yAxisRight, Qt::AlignRight);

  yAxisLeft->setLabelFormat("%d");
  yAxisLeft->setBase(10.0);
  yAxisLeft->setMinorTickCount(-1);
  yAxisLeft->setMax(1000);
  yAxisRight->setLabelFormat("%d");
  yAxisRight->setBase(10.0);
  yAxisRight->setMinorTickCount(-1);
  yAxisRight->setMax(1000);
}

void
SequentialSeriesScrollingChartView::enableFixedXAxis()
{
  assert(!mFixedXAxis && "enableFixedXAxis() can only be called once");

  mXAxis->setVisible(false);

  mFixedXAxis = new QValueAxis();
  mFixedXAxis->setLabelFormat("%d");
  mFixedXAxis->setTickCount(7);
  mChart->addAxis(mFixedXAxis, Qt::AlignBottom);

  refreshXRange();
}

void
SequentialSeriesScrollingChartView::setSeriesSpacing(const size_t& spacing)
{
  assert(mSeriesLists.size() == 0 &&
         "setSeriesSpacing() must only be called prior to adding datapoints");
  mSeriesSpacing = spacing;
}

void
SequentialSeriesScrollingChartView::setYAxisScalingMode(
  const YAxisScalingMode& mode)
{
  assert(dynamic_cast<QValueAxis*>(mYAxisLeft) &&
         "setYAxisScalingMode must not be called for log Y axes");
  assert(dynamic_cast<QValueAxis*>(mYAxisRight) &&
         "setYAxisScalingMode must not be called for log Y axes");
  mYAxisScalingMode = mode;
}

void
SequentialSeriesScrollingChartView::setTitle(const QString& title)
{
  mChart->setTitle(title);
}

void
SequentialSeriesScrollingChartView::setXRange(const qsizetype& range)
{
  mXRange = range;
  refreshXRange();
}

void
SequentialSeriesScrollingChartView::refreshXRange()
{
  const auto maxX = mSeriesLists[0].size() > 0
                      ? std::optional<qsizetype>(static_cast<qsizetype>(
                          mSeriesLists[0].back()->points().back().rx()))
                      : std::nullopt;
  if (!mFixedXAxis) {
    qsizetype ceil = mXRange;
    if (maxX.has_value()) {
      ceil = std::max(ceil, maxX.value());
    }
    mXAxis->setRange(ceil - mXRange, ceil);
  } else {
    mFixedXAxis->setRange(-mXRange, 0);
    if (maxX.has_value()) {
      mXAxis->setRange(maxX.value() - mXRange, maxX.value());
    }
  }
}

void
SequentialSeriesScrollingChartView::wipeData()
{
  for (auto seriesList : mSeriesLists) {
    for (QLineSeries* series : seriesList) {
      mChart->removeSeries(series);
      delete series;
    }
    seriesList.clear();
  }
  refreshXRange();
}

void
SequentialSeriesScrollingChartView::addDatapoint(const size_t& deviceIndex,
                                                 const size_t& seriesIndex,
                                                 const qreal& value)
{
  assert(deviceIndex < 2 && "only (up to) 2 devices are supported");
  assert((seriesIndex + 1 >= mSeriesLists[deviceIndex].size()) &&
         "must not append to prior (already completed) series");

  const QLineSeries* const lastPopulatedSeries =
    mSeriesLists[deviceIndex].size() > 0 ? mSeriesLists[deviceIndex].back()
                                         : nullptr;

  QLineSeries* series;
  while (seriesIndex >= mSeriesLists[deviceIndex].size()) {
    // Add in a loop, because the caller could have skipped one or more
    // intermediate series. We could make the vector sparse... (which is likely
    // to be confusing to manage), or we can just add some empty series which
    // makes things easier to reason about.
    series = mSeriesLists[deviceIndex].emplace_back(new QLineSeries());

    // Ordering is significant: Qt complains (and ignores the request) if
    // you try to attach an axis to a new series, if that axis is already
    // attached to another series which is attached to a chart, and if that
    // new series is not yet attached to the chart. (Translation:
    // addSeries(foo), before you foo->attachAxis.)
    mChart->addSeries(series);
    series->attachAxis(mXAxis);
    series->attachAxis(deviceIndex == 0 ? mYAxisLeft : mYAxisRight);
  }
  series = mSeriesLists[deviceIndex].back();

  qsizetype x;

  if (series->points().length() > 0) {
    x = series->points().back().rx() + 1;
  } else {
    if (lastPopulatedSeries) {
      x = lastPopulatedSeries->points().back().rx() + mSeriesSpacing;
    } else {
      // First point, ever.
      x = 0;
    }
  }

  series->append(QPoint(x, value));
  refreshXRange();

  // Using something functional to find the min/max Y values would certainly be
  // more elegant, but this is faster (at the cost of 16 bytes).
  qreal minYSeen, maxYSeen;
  if (deviceIndex == 0) {
    minYSeen = mMinYSeenLeft = std::min(mMinYSeenLeft, value);
    maxYSeen = mMaxYSeenLeft = std::max(mMaxYSeenLeft, value);
  } else {
    minYSeen = mMinYSeenRight = std::min(mMinYSeenRight, value);
    maxYSeen = mMaxYSeenRight = std::max(mMaxYSeenRight, value);
  }

  QAbstractAxis* const yAxis = deviceIndex == 0 ? mYAxisLeft : mYAxisRight;
  QLogValueAxis* const logYAxis = dynamic_cast<QLogValueAxis*>(yAxis);
  if (logYAxis == nullptr) {
    switch (mYAxisScalingMode) {
      case ZeroAnchored: {
        // TODO: configure making this configurable
        const qsizetype max = static_cast<qsizetype>(maxYSeen) / 20 * 20 + 20;
        yAxis->setRange(0.0, static_cast<qreal>(max));
        break;
      }
      case Floating: {
        // TODO: consider making margins configurable.
        const qsizetype min =
          std::max(static_cast<qsizetype>(0),
                   static_cast<qsizetype>(minYSeen) / 100 * 100 - 100);
        const qsizetype max =
          static_cast<qsizetype>(maxYSeen) / 100 * 100 + 200;
        yAxis->setRange(static_cast<qreal>(min), static_cast<qreal>(max));
        break;
      }
    }
  } else {
    // Round up to the next whole log as that seems easiest to read.
    auto maxLog = std::log10(maxYSeen);
    logYAxis->setMax(std::pow(10.0, std::max(std::floor(maxLog) + 1, 3.0)));
  }
}
