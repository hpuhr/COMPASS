#ifndef EVALUATIONDATAFILTERDIALOG_H
#define EVALUATIONDATAFILTERDIALOG_H

#include <QDialog>

class EvaluationData;
class EvaluationManager;

class QCheckBox;

class EvaluationDataFilterDialog  : public QDialog
{
    Q_OBJECT

public slots:
    void removeShortTargetsSlot(bool checked);

    void runSlot();
    void cancelSlot();

public:
    EvaluationDataFilterDialog(EvaluationData& eval_data, EvaluationManager& eval_man,
                               QWidget* parent=nullptr, Qt::WindowFlags f=0);

protected:
    EvaluationData& eval_data_;
    EvaluationManager& eval_man_;

    QCheckBox* remove_short_check_ {nullptr};

    //    unsigned int remove_short_targets_min_updates_ {10};
    //    double remove_short_targets_min_duration_ {60.0};

    //    bool remove_psr_only_targets_ {true};

    QPushButton* run_button_{nullptr};
    QPushButton* cancel_button_{nullptr};
};

#endif // EVALUATIONDATAFILTERDIALOG_H
