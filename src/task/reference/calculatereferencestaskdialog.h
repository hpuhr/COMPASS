#ifndef CALCULATEREFERENCESTASKDIALOG_H
#define CALCULATEREFERENCESTASKDIALOG_H


#include <QDialog>


class CalculateReferencesTask;

class QPushButton;
class QDoubleSpinBox;
class QCheckBox;
class QSpinBox;

class CalculateReferencesTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void runSignal();
    void cancelSignal();

public slots:
    void runClickedSlot();
    void cancelClickedSlot();

public:
    CalculateReferencesTaskDialog(CalculateReferencesTask& task);

    void updateButtons();

protected:
    void createUI();
    void createSettingsWidget(QWidget* w);
    QWidget* addScrollArea(QWidget* w) const;

    void readOptions();
    void writeOptions();

    CalculateReferencesTask& task_;

    QPushButton*    cancel_button_{nullptr};
    QPushButton*    run_button_   {nullptr};

    QDoubleSpinBox* R_std_box_      = nullptr;
    QDoubleSpinBox* R_std_high_box_ = nullptr;
    QDoubleSpinBox* Q_std_box_      = nullptr;
    QDoubleSpinBox* P_std_box_      = nullptr;
    QDoubleSpinBox* P_std_high_box_ = nullptr;

    QDoubleSpinBox* min_dt_box_         = nullptr;
    QDoubleSpinBox* max_dt_box_         = nullptr;
    QSpinBox*       min_chain_size_box_ = nullptr;

    QCheckBox*      use_vel_mm_box_ = nullptr;
    QCheckBox*      smooth_box_     = nullptr;

    QCheckBox*      resample_systracks_box_    = nullptr;
    QDoubleSpinBox* resample_systracks_dt_box_ = nullptr;

    QCheckBox*      resample_result_box_    = nullptr;
    QDoubleSpinBox* resample_result_dt_box_ = nullptr;

    QCheckBox*      verbose_box_ = nullptr;
    QCheckBox*      gen_vp_box_  = nullptr;
};

#endif // CALCULATEREFERENCESTASKDIALOG_H
