#include "exercisesmodel.h"

#include <QBrush>
#include <QFont>
#include <QIcon>
#include <QLabel>

#include "exerciserowwidget.h"

ExercisesModel::ExercisesModel(QObject* aParent)
  : QAbstractTableModel(aParent)
  , mExercises()
  , mCurrentExercise(-1)
  , mFitFactors()
  , mErrs()
  , mInterimFitFactors()
  , mCurrentExerciseLabel(nullptr)
{
}

void
ExercisesModel::setCurrentExerciseLabel(QLabel* const aCurrentExerciseLabel)
{
  mCurrentExerciseLabel = aCurrentExerciseLabel;
  refreshCurrentExerciseLabel();
}

void
ExercisesModel::refreshCurrentExerciseLabel()
{
  if (mCurrentExerciseLabel == nullptr) {
    return;
  }

  if (mCurrentExercise < 0 || mExercises.length() == 0) {
    mCurrentExerciseLabel->setText("");
    return;
  }

  QString message = QString("%1/%2: %3")
                      .arg(QString::number(mCurrentExercise + 1),
                           QString::number(mExercises.length()),
                           mExercises[mCurrentExercise]);
  mCurrentExerciseLabel->setText(message);
}

void
ExercisesModel::setExercises(const QStringList& aExercises)
{
  beginResetModel();
  mExercises = aExercises;
  mFitFactors = QList<double>(aExercises.length(), -1.0);
  mErrs = QList<double>(aExercises.length(), -1.0);
  mInterimFitFactors = QList<double>(aExercises.length(), -1.0);
  mCurrentExercise = 0;
  endResetModel();
  refreshCurrentExerciseLabel();
}

void
ExercisesModel::updateCurrentExercise(const uint aExercise)
{
  const uint previous = std::max(mCurrentExercise, 0);
  // TODO: make this robust against non-incremental changes.
  mCurrentExercise = aExercise;
  // Include all roles, because many styling-related properties change when
  // switching exercises.
  emit dataChanged(index(previous, 0), index(aExercise, 0));
  refreshCurrentExerciseLabel();
}

void
ExercisesModel::updateFF(const uint aExercise,
                         const double aFitFactor,
                         const double aErr)
{
  mFitFactors[aExercise] = aFitFactor;
  mErrs[aExercise] = aErr;
  emit dataChanged(index(aExercise, 0),
                   index(aExercise, 0),
                   QList<int>(Qt::DisplayRole, Qt::DecorationRole));
}

void
ExercisesModel::renderInterimFF(const uint aExercise, double aFitFactor)
{
  mInterimFitFactors[aExercise] = aFitFactor;
  emit dataChanged(index(aExercise, 0),
                   index(aExercise, 0),
                   QList<int>(Qt::DisplayRole, Qt::DecorationRole));
}

int
ExercisesModel::rowCount(const QModelIndex&) const
{
  return mExercises.length();
}

int
ExercisesModel::columnCount(const QModelIndex&) const
{
  return 1;
}

QVariant
ExercisesModel::data(const QModelIndex& aIndex, const int aRole) const
{
  auto row = aIndex.row();
  const bool isCurrentExercise =
    (row == mCurrentExercise && mFitFactors[mCurrentExercise] < 0);
  switch (aRole) {
    case Qt::DisplayRole: {
      // TODO: adapt this to handle multi-device results.
      bool isInterim = false;
      std::vector<std::optional<double>> ffs = {};
      std::vector<std::optional<double>> ffErrors = {};
      auto ff0 = mFitFactors[row];
      auto interimFF0 = mInterimFitFactors[row];
      if (ff0 >= 0) {
        ffs.push_back(ff0);
        ffErrors.push_back(mErrs[row]);
      } else if (interimFF0 >= 0) {
        ffs.push_back(interimFF0);
        isInterim = true;
      }
      return QVariant::fromValue(ExerciseData{
        .mTitle =
          QString("%1. %2").arg(QString::number(row + 1), mExercises[row]),
        .mFitFactors = ffs,
        .mFitFactorErrors = ffErrors,
        .mIsInterim = isInterim,
        .mIsRecording = isCurrentExercise,
      });
    }
    default:
      break;
  }
  return QVariant();
}
