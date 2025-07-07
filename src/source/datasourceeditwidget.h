#pragma once

#include <QWidget>

class DataSourceManager;
class DataSourcesConfigurationDialog;
class DSTypeSelectionComboBox;

class QLabel;
class QLineEdit;
class QPushButton;
class QGridLayout;
class QComboBox;

class DataSourceEditWidget : public QWidget
{
    Q_OBJECT

public slots:
    void nameEditedSlot(const QString& value);
    void shortNameEditedSlot(const QString& value);
    void dsTypeEditedSlot(const QString& value);

    void updateIntervalEditedSlot(const QString& value_str);

    void detectionTypeChangedSlot(int index); // Slot to handle detection type change

    void latitudeEditedSlot(const QString& value_str);
    void longitudeEditedSlot(const QString& value_str);
    void altitudeEditedSlot(const QString& value_str);

    void addRadarRangesSlot();
    void radarRangeEditedSlot(const QString& value_str);

    void addRadarAccuraciesSlot();
    void radarAccuraciesEditedSlot(const QString& value_str);

    void addNetLinesSlot();
    void netLineEditedSlot(const QString& value_str);

    void deleteSlot();

public:
    DataSourceEditWidget(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog);

    void showID(unsigned int ds_id);
    void clear();

    void updateContent();

protected:
    DataSourceManager& ds_man_;
    DataSourcesConfigurationDialog& dialog_;

    bool has_current_ds_ {false};
    unsigned int current_ds_id_ {0};
    bool current_ds_in_db_ {false};

    QLineEdit* name_edit_{nullptr};
    QLineEdit* short_name_edit_{nullptr};

    DSTypeSelectionComboBox* dstype_combo_{nullptr};

    QLabel* sac_label_{nullptr};
    QLabel* sic_label_{nullptr};
    QLabel* ds_id_label_{nullptr};

    // update_interval
    QLineEdit* update_interval_edit_{nullptr};

    QComboBox* detection_type_combo_{nullptr}; // << Add this line

    // position
    QWidget* position_widget_{nullptr};
    QLineEdit* latitude_edit_{nullptr};
    QLineEdit* longitude_edit_{nullptr};
    QLineEdit* altitude_edit_{nullptr};

    // radar ranges
    QWidget* ranges_widget_{nullptr};
    QLineEdit* psr_min_edit_{nullptr};
    QLineEdit* psr_max_edit_{nullptr};
    QLineEdit* ssr_min_edit_{nullptr};
    QLineEdit* ssr_max_edit_{nullptr};
    QLineEdit* mode_s_min_edit_{nullptr};
    QLineEdit* mode_s_max_edit_{nullptr};

    QPushButton* add_ranges_button_{nullptr};

    // radar accuracies
    QWidget* accuracies_widget_{nullptr};
    QLineEdit* acc_psr_azm_edit_{nullptr};
    QLineEdit* acc_psr_rng_edit_{nullptr};
    QLineEdit* acc_ssr_azm_edit_{nullptr};
    QLineEdit* acc_ssr_rng_edit_{nullptr};
    QLineEdit* acc_mode_s_azm_edit_{nullptr};
    QLineEdit* acc_mode_s_rng_edit_{nullptr};

    QPushButton* add_accuracies_button_{nullptr};

    // net lines
    QPushButton* add_lines_button_{nullptr};

    QWidget* net_widget_{nullptr};
    std::map<std::string, std::vector<QLineEdit*>> net_edits_; // L1 -> edits (listen, mcastip, mcastport, sender)

    QPushButton* delete_button_{nullptr};
};

