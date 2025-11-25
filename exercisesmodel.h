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
  void setFailLabel(QLabel* const aFailLabel);
  void setExercises(const QStringList& aExercises);

public slots:
  void updateCurrentExercise(const size_t& aExercise);
  void updateFF(const size_t& aDeviceIndex, const size_t& aExercise, const double& aFitFactor, const double& aErr);
  void renderInterimFF(const size_t& aDeviceIndex,
                       const size_t& aExercise,
                       const double& aFitFactor);
  void doTestStarted();
  void doTestCompleted();
  void doTestCancelled();

private:
  bool mIsRunning;

  QStringList mExercises;
  int mCurrentExercise;
  QList<double> mFitFactors;
  QList<double> mErrs;
  QList<double> mInterimFitFactors;
  QLabel* mCurrentExerciseLabel;
  QLabel* mFailLabel;

  void refreshCurrentExerciseLabel();
  void refreshFailLabel();
};

#endif // EXERCISESMODEL_H
