#ifndef EXERCISESMODEL_H
#define EXERCISESMODEL_H

#include <QAbstractTableModel>

class QLabel;

class ExercisesModel : public QAbstractTableModel
{
public:
  explicit ExercisesModel(QObject* aParent = nullptr);
  int rowCount(const QModelIndex& aParent) const override;
  int columnCount(const QModelIndex& aParent) const override;
  QVariant data(const QModelIndex& aIndex, const int aRole) const override;

  void setCurrentExerciseLabel(QLabel* const aCurrentExerciseLabel);
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
  QLabel* mCurrentExerciseLabel;

  void refreshCurrentExerciseLabel();
};

#endif // EXERCISESMODEL_H
