#pragma once

#include "timewindowcollectionwidget.h"

#include <QDialog>

class EvaluationManager;

class QCheckBox;
class QDateTimeEdit;

class EvaluationTimestampConditionsDialog : public QDialog
{
    Q_OBJECT

private slots:
    void toggleUseTimeSlot();
    void timeBeginEditedSlot (const QDateTime& datetime);
    void timeEndEditedSlot (const QDateTime& datetime);

public:
    EvaluationTimestampConditionsDialog(EvaluationManager& eval_man, QWidget* parent=nullptr);
    virtual ~EvaluationTimestampConditionsDialog();

    bool somethingChangedFlag() const;

protected:
    EvaluationManager& eval_man_;

    QCheckBox* use_time_check_{nullptr};
    QDateTimeEdit* time_begin_edit_{nullptr};
    QDateTimeEdit* time_end_edit_{nullptr};

    TimeWindowCollectionWidget* tw_widget_{nullptr};

    bool update_active_ {false};
    bool something_changed_flag_ {false};

    void updateValues();

};

