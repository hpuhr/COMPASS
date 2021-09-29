#ifndef EVALUATIONDATAFILTERDIALOG_H
#define EVALUATIONDATAFILTERDIALOG_H

#include <QDialog>

#include <map>

class EvaluationData;
class EvaluationManager;

class QCheckBox;
class QLineEdit;
class QTextEdit;

class EvaluationDataFilterDialog  : public QDialog
{
    Q_OBJECT

public slots:
    // shorts
    void removeShortTargetsSlot(bool checked);
    void removeSTMinUpdatesEditedSlot();
    void removeSTMinDurationEditedSlot();
    // psr
    void removePSROnlyTargetsSlot(bool checked);
    // ma
    void removeModeACOnlyTargetsSlot(bool checked);
    void removeModeASlot(bool checked);
    void removeModeAValuesSlot();
    // mc
    void removeModeCSlot(bool checked);
    void removeModeCMinValueSlot();
    //ta
    void removeTASlot(bool checked);
    void removeTAValuesSlot();
    // dbo
    void removeDBOsSlot(bool checked);
    void removeSpecificDBOsSlot(bool checked);

    void runSlot();
    void cancelSlot();

public:
    EvaluationDataFilterDialog(EvaluationData& eval_data, EvaluationManager& eval_man,
                               QWidget* parent=nullptr, Qt::WindowFlags f=0);

protected:
    EvaluationData& eval_data_;
    EvaluationManager& eval_man_;

    QCheckBox* remove_short_check_ {nullptr};

    QLineEdit* remove_st_min_updates_edit_ {nullptr};
    QLineEdit* remove_st_min_duration_edit_ {nullptr};

    QCheckBox* remove_psr_only_targets_check_ {nullptr};

    QCheckBox* remove_mode_ac_only_check_ {nullptr};
    QCheckBox* remove_mode_a_check_ {nullptr};
    QTextEdit* remove_mode_a_edit_ {nullptr};

    QCheckBox* remove_mode_c_check_ {nullptr};
    QTextEdit* remove_mode_c_min_edit_ {nullptr};


    QCheckBox* remove_ta_check_ {nullptr};
    QTextEdit* remove_ta_edit_ {nullptr};

    QCheckBox* remove_dbo_check_ {nullptr};
    std::map<std::string, QCheckBox*> remove_dbo_checks_;

    QPushButton* run_button_{nullptr};
    QPushButton* cancel_button_{nullptr};

};

#endif // EVALUATIONDATAFILTERDIALOG_H
