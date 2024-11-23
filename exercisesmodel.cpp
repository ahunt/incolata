#include "exercisesmodel.h"

#include <QBrush>
#include <QFont>
#include <QIcon>

ExercisesModel::ExercisesModel(QObject* parent)
  : QAbstractTableModel{ parent }
  , exercises(QStringList() << "...")
  , currentExercise(0)
  , ffs(1, -1.0)
  , errs(1, -1.0)
  , interimFFs(1, -1.0)
{
}

void ExercisesModel::setExercises(const QStringList& newExercises) {
  beginResetModel();
  this->exercises = newExercises;
  this->ffs = QList<double>(newExercises.length(), -1.0);
  this->errs = QList<double>(newExercises.length(), -1.0);
  this->interimFFs = QList<double>(newExercises.length(), -1.0);
  endResetModel();
}

void
ExercisesModel::updateCurrentExercise(uint ex)
{
  uint previous = std::max(currentExercise, 0);
  // TODO: make this robust against non-incremental changes.
  currentExercise = ex;
  // TODO: consider passing in list of roles too?
  emit dataChanged(index(previous, 0), index(ex, 2));
}

void
ExercisesModel::updateFF(uint ex, double ff, double err)
{
  ffs[ex] = ff;
  errs[ex] = err;
  // TODO: set correct role too.
  emit dataChanged(index(ex, 2), index(ex, 2));
}

void
ExercisesModel::renderInterimFF(uint ex, double ff)
{
  interimFFs[ex] = ff;
  // TODO: set correct role too.
  emit dataChanged(index(ex, 2), index(ex, 2));
}

int
ExercisesModel::rowCount(const QModelIndex&) const
{
  return exercises.length();
}

int
ExercisesModel::columnCount(const QModelIndex&) const
{
  return 3;
}

QVariant
ExercisesModel::data(const QModelIndex& index, int role) const
{
  switch (role) {
    case Qt::DisplayRole:
      if (index.column() == 1) {
        return exercises[index.row()];
      } else if (index.column() == 2) {
        double finalFF = ffs[index.row()];
        double interimFF = interimFFs[index.row()];
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
	  s << "±" << QString::number(errs[index.row()], 'f', 1);
        }
        s << " / " << QString::number(log10(ff), 'f', 2)
          << QStringLiteral(" log₁₀");
        return result;
      }
      break;
    case Qt::DecorationRole:
      // Icon names from
      // https://specifications.freedesktop.org/icon-naming-spec/latest/
      if (index.column() == 0 && index.row() == currentExercise) {
        return QIcon::fromTheme("media-playback-start");
      } else if (index.column() == 2) {
        // We could perform this check for all prior rows, but we only expect
        // this condition to ever be true for the immediately preceeding row.
        if ((index.row() == currentExercise ||
             index.row() == currentExercise - 1) &&
            ffs[index.row()] < 0) {
          return QIcon::fromTheme("system-search");
        }
      }
      break;
    case Qt::FontRole:
      if (index.column() == 1 && index.row() == currentExercise) {
        QFont font;
        font.setBold(true);
        font.setPointSize(font.pointSize() + 2);
        return font;
      }
      break;
    case Qt::BackgroundRole:
      if (index.row() == currentExercise) {
        return QBrush(QColorConstants::Yellow);
      }
      break;
  }
  return QVariant();
}
