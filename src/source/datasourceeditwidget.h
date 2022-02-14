#ifndef DATASOURCEEDITWIDGET_H
#define DATASOURCEEDITWIDGET_H

#include <QWidget>

class DataSourceManager;
class DataSourcesConfigurationDialog;

class QLineEdit;
class QPushButton;

class DataSourceEditWidget : public QWidget
{
    Q_OBJECT

public:
    DataSourceEditWidget(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog);

    void showID(unsigned int ds_id);
    void clear();

protected:
    DataSourceManager& ds_man_;
    DataSourcesConfigurationDialog& dialog_;

    bool has_current_ds_ {false};
    unsigned int current_ds_id_ {0};
    bool current_ds_in_db_ {false};

    QLineEdit* name_edit_{nullptr};
    QLineEdit* short_name_edit_{nullptr};

    void updateContent();

};

#endif // DATASOURCEEDITWIDGET_H
