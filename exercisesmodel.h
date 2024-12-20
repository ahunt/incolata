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

  void setExercises(const QStringList& exercises);

public slots:
  void updateCurrentExercise(uint ex);
  void updateFF(uint ex, double ff, double err);
  void renderInterimFF(uint ex, double ff);

private:
  QStringList exercises;
  int currentExercise;
  QList<double> ffs;
  QList<double> errs;
  QList<double> interimFFs;
};

#endif // EXERCISESMODEL_H
