
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
class QLineEdit;

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
    void createPositionFilterSettingsWidget(QWidget* w);
    void createKalmanSettingsWidget(QWidget* w);
    void createOutputSettingsWidget(QWidget* w);
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

    // position filters

    QCheckBox* filter_position_usage_check_ {nullptr};
    //    bool filter_position_usage {true};

    // tracker position usage
    QCheckBox* tracker_only_confirmed_positions_check_ {nullptr};
    //    bool tracker_only_confirmed_positions {true}; // non-tentative
    QCheckBox* tracker_only_noncoasting_positions_check_ {nullptr};
    //    bool tracker_only_noncoasting_positions {true};
    QCheckBox* tracker_only_report_detection_positions_check_ {nullptr};
    //    bool tracker_only_report_detection_positions {false}; // no no detection
    QCheckBox* tracker_only_report_detection_nonpsronly_positions_check_ {nullptr};
    //    bool tracker_only_report_detection_nonpsronly_positions {true}; // no mono + psr det
    QCheckBox* tracker_only_high_accuracy_postions_check_ {nullptr};
    //    bool tracker_only_high_accuracy_postions {true};
    QDoubleSpinBox* tracker_minimum_accuracy_box_ {nullptr};
    //    float tracker_minimum_accuracy {30}; // m

    //    // adsb position usage
    QCheckBox* adsb_only_v12_positions_check_ {nullptr};
    //    bool adsb_only_v12_positions {true};

    QCheckBox* adsb_only_high_nucp_nic_positions_check_ {nullptr};
    //    bool adsb_only_high_nucp_nic_positions {true};
    QSpinBox* adsb_minimum_nucp_nic_box_ {nullptr};
    //    unsigned int adsb_minimum_nucp_nic {4};

    QCheckBox* adsb_only_high_nacp_positions_check_ {nullptr};
    //    bool adsb_only_high_nacp_positions {false};
    QSpinBox* adsb_minimum_nacp_box_ {nullptr};
    //    unsigned int adsb_minimum_nacp {4};

    QCheckBox* adsb_only_high_sil_positionss_check_ {nullptr};
    //    bool adsb_only_high_sil_positions {false};
    QSpinBox* adsb_minimum_sil_box_ {nullptr};
    //    unsigned int adsb_minimum_sil {1};

    // kalman
    QComboBox*      rec_type_box_ = nullptr;
    QComboBox*      map_mode_box_ = nullptr;

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

    // output source
    QLineEdit* ds_name_edit_ {nullptr};
    QSpinBox* ds_sac_box_ {nullptr};
    QSpinBox* ds_sic_box_ {nullptr};
    QComboBox* ds_line_box_ {nullptr};
};

#endif // CALCULATEREFERENCESTASKDIALOG_H
