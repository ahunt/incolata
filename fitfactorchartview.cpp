#include "fitfactorchartview.h"

#include <QLineSeries>
#include <QLogValueAxis>
#include <QValueAxis>

FitFactorChartView::FitFactorChartView(QWidget* const parent)
  : QChartView(parent)
  , mExerciseCount(8)
  , mMaxFFSeen(0)
  , mSeriess(2)
  , mXAxis(new QValueAxis())
  , mYAxis(new QLogValueAxis())
{
  chart()->setTitle("Fit Factor");
  chart()->legend()->hide();

  mXAxis->setLabelFormat("%d");
  mXAxis->setTickType(QValueAxis::TicksDynamic);
  mXAxis->setTickInterval(1);

  mYAxis->setLabelFormat("%d");
  mYAxis->setMax(1000);
  mYAxis->setBase(10.0);
  mYAxis->setMinorTickCount(-1);

  chart()->addSeries(mSeriess[0]);
  chart()->addSeries(mSeriess[1]);
  chart()->addAxis(mXAxis, Qt::AlignBottom);
  chart()->addAxis(mYAxis, Qt::AlignLeft);

  for (size_t i = 0; i < 2; i++) {
    mSeriess[i]->setPointsVisible(true);
    mSeriess[i]->attachAxis(mXAxis);
    mSeriess[i]->attachAxis(mYAxis);
  }

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
FitFactorChartView::wipeData()
{
  mSeriess[0]->removePoints(0, mSeriess[0]->count());
  mSeriess[1]->removePoints(0, mSeriess[1]->count());
  mMaxFFSeen = 0;
  // Do not reset y-axis max: there's no point, and there's a non-zero
  // probability that this test will have the same max as the last test (e.g. if
  // the subject is repeatedly testing the same specimen).
}

void
FitFactorChartView::refreshExerciseRange()
{
  mXAxis->setRange(0.5, mExerciseCount + 0.5);
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
  gradient.setColorAt(std::log(100 - stopMargin) / maxLog, QRgb(0xFFBBBB));
  gradient.setColorAt(std::log(100 + stopMargin) / maxLog, QRgb(0xFFFFBB));
  gradient.setColorAt(std::log(200 - stopMargin) / maxLog, QRgb(0xFFFFBB));
  gradient.setColorAt(std::log(200 + stopMargin) / maxLog, QRgb(0xCCFFCC));

  chart()->setPlotAreaBackgroundBrush(gradient);
  chart()->setPlotAreaBackgroundVisible(true);
}

void
FitFactorChartView::addDatapoint(const size_t& deviceIndex,
                                 const size_t& exercise,
                                 const double& fitFactor)
{
  assert(exercise <
           static_cast<size_t>(std::numeric_limits<qsizetype>::max()) + 1 &&
         "sorry, FTL doesn't support that many exercises");

  mSeriess[deviceIndex]->append(QPoint(exercise + 1, fitFactor));

  // Round up to the next whole log as that seems easiest to read.
  mMaxFFSeen = std::max(mMaxFFSeen, fitFactor);
  auto maxLog = std::log10(mMaxFFSeen);
  mYAxis->setMax(std::pow(10.0, std::max(std::floor(maxLog) + 1, 3.0)));

  refreshBackground();
}
