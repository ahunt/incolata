#ifndef EXERCISESMODEL_H
#define EXERCISESMODEL_H

#include <QAbstractTableModel>

class ExercisesModel : public QAbstractTableModel
{
public:
  explicit ExercisesModel(QObject* parent = nullptr);
  int rowCount(const QModelIndex& parent) const;
  int columnCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;
};

#endif // EXERCISESMODEL_H