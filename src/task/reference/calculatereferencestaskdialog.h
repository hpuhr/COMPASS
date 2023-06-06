#ifndef CALCULATEREFERENCESTASKDIALOG_H
#define CALCULATEREFERENCESTASKDIALOG_H


#include <QDialog>

class CalculateReferencesTask;
class SelectDataSourcesWidget;

class QPushButton;
class QDoubleSpinBox;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QTabWidget;

class CalculateReferencesTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void runSignal();
    void cancelSignal();

public slots:
    void runClickedSlot();
    void cancelClickedSlot();

    void toggleTrackerSourcesSlot();
    void trackerSourcesChangedSlot(std::map<std::string, bool> selection);
    void toggleADSBSourcesSlot();
    void adsbSourcesChangedSlot(std::map<std::string, bool> selection);

public:
    CalculateReferencesTaskDialog(CalculateReferencesTask& task);

    void updateSourcesWidgets(); // disables if not selected
    void updateButtons();

protected:
    void createUI();
    void createDataSourcesSettingsWidget(QWidget* w);
    void createFilterSettingsWidget(QWidget* w);
    void createKalmanSettingsWidget(QWidget* w);
    QWidget* addScrollArea(QWidget* w) const;

    void readOptions();
    void writeOptions();

    CalculateReferencesTask& task_;

    QTabWidget* tab_widget_{nullptr};

    // buttons
    QPushButton*    cancel_button_{nullptr};
    QPushButton*    run_button_   {nullptr};

    // data sources

    QCheckBox* use_tracker_check_ {nullptr};
    SelectDataSourcesWidget* tracker_sources_ {nullptr};
    QCheckBox* use_adsb_check_ {nullptr};
    SelectDataSourcesWidget* adsb_sources_ {nullptr};

    // filters

    // kalman
    QComboBox*      rec_type_box_ = nullptr;

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

    QCheckBox*      verbose_box_     = nullptr;
    QCheckBox*      python_comp_box_ = nullptr;
};

#endif // CALCULATEREFERENCESTASKDIALOG_H
