#ifndef EXERCISESMODEL_H
#define EXERCISESMODEL_H

#include <QAbstractTableModel>

class ExercisesModel : public QAbstractTableModel
{
public:
  explicit ExercisesModel(QObject* aParent = nullptr);
  int rowCount(const QModelIndex& aParent) const;
  int columnCount(const QModelIndex& aParent) const;
  QVariant data(const QModelIndex& aIndex, const int aRole) const;

  void setExercises(const QStringList& aExercises);

public slots:
  void updateCurrentExercise(const uint aExercise);
  void updateFF(const uint aExercise, const double aFitFactor, double aErr);
  void renderInterimFF(uint aExercise, double aFitFactor);

private:
  QStringList mExercises;
  int mCurrentExercise;
  QList<double> mFitFactors;
  QList<double> mErrs;
  QList<double> mInterimFitFactors;
};

#endif // EXERCISESMODEL_H
