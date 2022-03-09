#ifndef DATASOURCEEDITWIDGET_H
#define DATASOURCEEDITWIDGET_H

#include <QWidget>

class DataSourceManager;
class DataSourcesConfigurationDialog;
class DSTypeSelectionComboBox;

class QLabel;
class QLineEdit;
class QPushButton;
class QGridLayout;

class DataSourceEditWidget : public QWidget
{
    Q_OBJECT

public slots:
    void nameEditedSlot(const QString& value);
    void shortNameEditedSlot(const QString& value);
    void dsTypeEditedSlot(const QString& value);

    void latitudeEditedSlot(const QString& value_str);
    void longitudeEditedSlot(const QString& value_str);
    void altitudeEditedSlot(const QString& value_str);

    void net1EditedSlot(const QString& value_str);
    void net2EditedSlot(const QString& value_str);
    void net3EditedSlot(const QString& value_str);
    void net4EditedSlot(const QString& value_str);

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

    QWidget* position_widget_{nullptr};
    QLineEdit* latitude_edit_{nullptr};
    QLineEdit* longitude_edit_{nullptr};
    QLineEdit* altitude_edit_{nullptr};

    QWidget* net_widget_{nullptr};
    QLineEdit* net_l1_edit_{nullptr};
    QLineEdit* net_l2_edit_{nullptr};
    QLineEdit* net_l3_edit_{nullptr};
    QLineEdit* net_l4_edit_{nullptr};

    QPushButton* delete_button_{nullptr};
};

#endif // DATASOURCEEDITWIDGET_H
