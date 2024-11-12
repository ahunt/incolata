#include "fitfactorchartview.h"

#include <QLineSeries>
#include <QLogValueAxis>
#include <QValueAxis>

FitFactorChartView::FitFactorChartView(QWidget* const parent)
  : QChartView(parent)
  , mExerciseCount(8)
  , mMaxFFSeen(0)
  , mSeries(new QLineSeries())
  , mXAxis(new QValueAxis())
  , mYAxis(new QLogValueAxis())
{
  chart()->setTitle("Fit Factors");
  chart()->legend()->hide();

  mXAxis->setLabelFormat("%d");
  mXAxis->setTickType(QValueAxis::TicksDynamic);
  // Only label every second tick for space reasons.
  // TODO: consider making this dynamic depending on available width?
  mXAxis->setTickInterval(2);
  mXAxis->setTickAnchor(1);
  mXAxis->setMinorTickCount(1);

  mYAxis->setLabelFormat("%d");
  mYAxis->setMax(1000);
  mYAxis->setBase(10.0);
  mYAxis->setMinorTickCount(-1);

  chart()->addSeries(mSeries);
  chart()->addAxis(mXAxis, Qt::AlignBottom);
  chart()->addAxis(mYAxis, Qt::AlignLeft);

  mSeries->setPointsVisible(true);
  mSeries->attachAxis(mXAxis);
  mSeries->attachAxis(mYAxis);

  chart()->setMargins(QMargins(5,5,5,5));

  refreshExerciseRange();
  refreshBackground();
}

void
FitFactorChartView::setExerciseCount(const qsizetype& count)
{
  mExerciseCount = count;
  refreshExerciseRange();
}

void
FitFactorChartView::refreshExerciseRange()
{
  // 0 is deliberately included to provide an empty margin. (I'm not entirely
  // convinced that this is the best UX yet.)
  mXAxis->setRange(0, mExerciseCount);
}

void
FitFactorChartView::refreshBackground()
{
  QLinearGradient gradient;
  gradient.setStart(0.0, 1.0);
  gradient.setFinalStop(0.0, 0.0);
  gradient.setCoordinateMode(QGradient::ObjectMode);

  auto maxLog = std::log(mYAxis->max());
  auto stopMargin = 30;
  gradient.setColorAt(std::log(100 - stopMargin) / maxLog, QRgb(0xFF0000));
  gradient.setColorAt(std::log(100 + stopMargin) / maxLog, QRgb(0xFFFF00));
  gradient.setColorAt(std::log(200 - stopMargin) / maxLog, QRgb(0xFFFF00));
  gradient.setColorAt(std::log(200 + stopMargin) / maxLog, QRgb(0xBBFFBB));

  chart()->setPlotAreaBackgroundBrush(gradient);
  chart()->setPlotAreaBackgroundVisible(true);
}

void
FitFactorChartView::addDatapoint(const size_t& exercise,
                                 const double& fitFactor)
{
  assert(exercise <
           static_cast<size_t>(std::numeric_limits<qsizetype>::max()) + 1 &&
         "sorry, FTL doesn't support that many exercises");

  mSeries->append(QPoint(exercise + 1, fitFactor));

  // Round up to the next whole log as that seems easiest to read.
  mMaxFFSeen = std::max(mMaxFFSeen, fitFactor);
  auto maxLog = std::log10(mMaxFFSeen);
  mYAxis->setMax(std::pow(10.0, std::floor(maxLog) + 1));

  refreshBackground();
}
