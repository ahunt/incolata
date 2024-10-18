#include "exercisesmodel.h"

#include <QBrush>
#include <QFont>
#include <QIcon>

ExercisesModel::ExercisesModel(QObject* parent)
  : QAbstractTableModel{ parent }
{
}

int
ExercisesModel::rowCount(const QModelIndex&) const
{
  return 8;
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
        return QVariant("abc");
      } else if (index.column() == 2 && index.row() < 2) {
        return QVariant("101");
      }
      break;
    case Qt::DecorationRole:
      if (index.column() == 0 && index.row() == 2) {
        return QIcon::fromTheme("media-playback-start");
      }
      break;
    case Qt::FontRole:
      if (index.column() == 1 && index.row() == 2) {
        QFont font;
        font.setBold(true);
        font.setPointSize(font.pointSize() + 2);
        return font;
      }
      break;
    case Qt::BackgroundRole:
      if (index.row() == 2) {
        return QBrush(QColorConstants::Yellow);
      }
      break;
  }
  return QVariant();
}
