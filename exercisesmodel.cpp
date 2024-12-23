#include "exercisesmodel.h"

#include <QBrush>
#include <QFont>
#include <QIcon>

ExercisesModel::ExercisesModel(QObject* aParent)
  : QAbstractTableModel(aParent)
  , mExercises()
  , mCurrentExercise(-1)
  , mFitFactors()
  , mErrs()
  , mInterimFitFactors()
{
}

void
ExercisesModel::setExercises(const QStringList& aExercises)
{
  beginResetModel();
  this->mExercises = aExercises;
  this->mFitFactors = QList<double>(aExercises.length(), -1.0);
  this->mErrs = QList<double>(aExercises.length(), -1.0);
  this->mInterimFitFactors = QList<double>(aExercises.length(), -1.0);
  endResetModel();
}

void
ExercisesModel::updateCurrentExercise(const uint aExercise)
{
  const uint previous = std::max(mCurrentExercise, 0);
  // TODO: make this robust against non-incremental changes.
  mCurrentExercise = aExercise;
  // TODO: consider passing in list of roles too?
  emit dataChanged(index(previous, 0), index(aExercise, 2));
}

void
ExercisesModel::updateFF(const uint aExercise,
                         const double aFitFactor,
                         const double aErr)
{
  mFitFactors[aExercise] = aFitFactor;
  mErrs[aExercise] = aErr;
  // TODO: set correct role too.
  emit dataChanged(index(aExercise, 2), index(aExercise, 2));
}

void
ExercisesModel::renderInterimFF(const uint aExercise, double aFitFactor)
{
  mInterimFitFactors[aExercise] = aFitFactor;
  // TODO: set correct role too.
  emit dataChanged(index(aExercise, 2), index(aExercise, 2));
}

int
ExercisesModel::rowCount(const QModelIndex&) const
{
  return mExercises.length();
}

int
ExercisesModel::columnCount(const QModelIndex&) const
{
  return 3;
}

QVariant
ExercisesModel::data(const QModelIndex& aIndex, const int aRole) const
{
  const bool isCurrentExercise =
    (aIndex.row() == mCurrentExercise && mFitFactors[mCurrentExercise] < 0);
  switch (aRole) {
    case Qt::DisplayRole:
      if (aIndex.column() == 1) {
        return mExercises[aIndex.row()];
      } else if (aIndex.column() == 2) {
        double finalFF = mFitFactors[aIndex.row()];
        double interimFF = mInterimFitFactors[aIndex.row()];
        // In a spherical world, a valid ff is always >= 1. But it's entirely
        // possible to achieve < 1 if you try to fool the machine and/or you
        // have a bad machine.
        if (finalFF < 0 && interimFF < 0) {
          return QVariant("");
        }
        bool isInterim = finalFF < 0;
        double ff = isInterim ? interimFF : finalFF;

        QString result;
        QTextStream s(&result);
        if (isInterim) {
          s << "<Interim> ";
        }
        if (ff >= 10) {
          s << QString::number(ff, 'f', 0);
        } else {
          s << QString::number(ff, 'f', 1);
        }
	if (!isInterim) {
          s << "±" << QString::number(mErrs[aIndex.row()], 'f', 1);
        }
        s << " / " << QString::number(log10(ff), 'f', 2)
          << QStringLiteral(" log₁₀");
        return result;
      }
      break;
    case Qt::DecorationRole:
      // Icon names from
      // https://specifications.freedesktop.org/icon-naming-spec/latest/
      if (aIndex.column() == 0 && isCurrentExercise) {
        return QIcon::fromTheme("media-playback-start");
      } else if (aIndex.column() == 2) {
        if (isCurrentExercise && mInterimFitFactors[aIndex.row()] < 0) {
          return QIcon::fromTheme("system-search");
        }
      }
      break;
    case Qt::FontRole:
      if (aIndex.column() == 1 && isCurrentExercise) {
        QFont font;
        font.setBold(true);
        font.setPointSize(font.pointSize() + 2);
        return font;
      }
      break;
    case Qt::BackgroundRole:
      if (isCurrentExercise) {
        return QBrush(QColorConstants::Yellow);
      }
      break;
  }
  return QVariant();
}
