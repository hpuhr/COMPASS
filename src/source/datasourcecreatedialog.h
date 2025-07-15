#pragma once

#include <QDialog>

class DataSourceManager;
class DataSourcesConfigurationDialog;
class DSTypeSelectionComboBox;

class QLineEdit;
class QPushButton;

class DataSourceCreateDialog : public QDialog
{
    Q_OBJECT

signals:
    void doneSignal();

public slots:
    void dsTypeEditedSlot(const QString& value);

    void sacEditedSlot(const QString& value_str);
    void sicEditedSlot(const QString& value_str);

    void cancelClickedSlot();
    void doneClickedSlot();

public:
    DataSourceCreateDialog(DataSourcesConfigurationDialog& dialog, DataSourceManager& ds_man);
    virtual ~DataSourceCreateDialog();

    unsigned int sac() const;

    unsigned int sic() const;

    std::string dsType() const;

    bool cancelled() const;

protected:
    DataSourceManager& ds_man_;

    DSTypeSelectionComboBox* dstype_combo_{nullptr};

    QLineEdit* sac_edit_{nullptr};
    QLineEdit* sic_edit_{nullptr};

    QPushButton* cancel_button_{nullptr};
    QPushButton* done_button_{nullptr};

    std::string ds_type_;
    unsigned int sac_ {0};
    unsigned int sic_ {0};

    bool cancelled_{false};

    void checkInput();
};

