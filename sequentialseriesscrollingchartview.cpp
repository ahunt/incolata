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
  , mFixedXAxis(nullptr)
  , mSeriesList()
  , mYAxisScalingMode(YAxisScalingMode::ZeroAnchored)
  , mSeriesSpacing(0)
  , mMinYSeen(std::numeric_limits<double>::infinity())
  , mMaxYSeen(0.0)
{
  QValueAxis* const yAxis = new QValueAxis();
  mYAxis = yAxis;

  mXAxis->setLabelFormat("%d");
  yAxis->setLabelFormat("%d");

  mXAxis->setTickType(QValueAxis::TicksDynamic);
  mXAxis->setTickAnchor(0);
  mXAxis->setTickInterval(10);

  mChart->legend()->hide();
  mChart->addAxis(mXAxis, Qt::AlignBottom);
  mChart->addAxis(mYAxis, Qt::AlignLeft);
}

QValueAxis*
SequentialSeriesScrollingChartView::xAxis() const
{
  return mXAxis;
}

QAbstractAxis*
SequentialSeriesScrollingChartView::yAxis() const
{
  return mYAxis;
}

void
SequentialSeriesScrollingChartView::enableLogYAxis()
{
  assert(mSeriesList.size() == 0 &&
         "enableLogYAxis() must only be called prior to adding datapoints");

  mChart->removeAxis(mYAxis);
  delete mYAxis;
  QLogValueAxis* const axis = new QLogValueAxis();
  mYAxis = axis;

  mChart->addAxis(mYAxis, Qt::AlignLeft);

  axis->setLabelFormat("%d");
  axis->setBase(10.0);
  axis->setMinorTickCount(-1);
  axis->setMax(1000);
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
  assert(mSeriesList.size() == 0 &&
         "setSeriesSpacing() must only be called prior to adding datapoints");
  mSeriesSpacing = spacing;
}

void
SequentialSeriesScrollingChartView::setYAxisScalingMode(
  const YAxisScalingMode& mode)
{
  assert(dynamic_cast<QValueAxis*>(mYAxis) &&
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
  const auto maxX = mSeriesList.size() > 0
                      ? std::optional<qsizetype>(static_cast<qsizetype>(
                          mSeriesList.back()->points().back().rx()))
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
SequentialSeriesScrollingChartView::addDatapoint(const size_t& seriesIndex,
                                                 const qreal& value)
{
  assert((seriesIndex + 1 >= mSeriesList.size()) &&
         "must not append to prior (already completed) series");

  const QLineSeries* const lastPopulatedSeries =
    mSeriesList.size() > 0 ? mSeriesList.back() : nullptr;

  QLineSeries* series;
  while (seriesIndex >= mSeriesList.size()) {
    // Add in a loop, because the caller could have skipped one or more
    // intermediate series. We could make the vector sparse... (which is likely
    // to be confusing to manage), or we can just add some empty series which
    // makes things easier to reason about.
    series = mSeriesList.emplace_back(new QLineSeries());

    // Ordering is significant: Qt complains (and ignores the request) if
    // you try to attach an axis to a new series, if that axis is already
    // attached to another series which is attached to a chart, and if that
    // new series is not yet attached to the chart. (Translation:
    // addSeries(foo), before you foo->attachAxis.)
    mChart->addSeries(series);
    series->attachAxis(mXAxis);
    series->attachAxis(mYAxis);
  }
  series = mSeriesList.back();

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
  mMinYSeen = std::min(mMinYSeen, value);
  mMaxYSeen = std::max(mMaxYSeen, value);

  QLogValueAxis* const logYAxis = dynamic_cast<QLogValueAxis*>(mYAxis);
  if (logYAxis == nullptr) {
    switch (mYAxisScalingMode) {
      case ZeroAnchored: {
        // TODO: configure making this configurable
        const qsizetype max = static_cast<qsizetype>(mMaxYSeen) / 20 * 20 + 20;
        mYAxis->setRange(0.0, static_cast<qreal>(max));
        break;
      }
      case Floating: {
        // TODO: consider making margins configurable.
        const qsizetype min =
          std::max(static_cast<qsizetype>(0),
                   static_cast<qsizetype>(mMinYSeen) / 100 * 100 - 100);
        const qsizetype max =
          static_cast<qsizetype>(mMaxYSeen) / 100 * 100 + 200;
        mYAxis->setRange(static_cast<qreal>(min), static_cast<qreal>(max));
        break;
      }
    }
  } else {
    // Round up to the next whole log as that seems easiest to read.
    auto maxLog = std::log10(mMaxYSeen);
    logYAxis->setMax(std::pow(10.0, std::max(std::floor(maxLog) + 1, 3.0)));
  }
}
