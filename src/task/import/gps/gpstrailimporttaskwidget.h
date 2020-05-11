#ifndef GPSTRAILIMPORTTASKWIDGET_H
#define GPSTRAILIMPORTTASKWIDGET_H

#include <taskwidget.h>

class GPSTrailImportTask;

class QPushButton;
class QListWidget;
class QTextEdit;
class QTabWidget;
class QHBoxLayout;
class QLineEdit;
class QCheckBox;

class GPSTrailImportTaskWidget : public TaskWidget
{
    Q_OBJECT
  public slots:
    void addFileSlot();
    void deleteFileSlot();
    void deleteAllFilesSlot();
    void selectedFileSlot();
    void updateFileListSlot();

    void sacEditedSlot(const QString& value);
    void sicEditedSlot(const QString& value);
    void nameEditedSlot(const QString& value);

    void todOffsetEditedSlot(const QString& value);

    void mode3ACheckedSlot();
    void mode3AEditedSlot(const QString& value);

    void targetAddressCheckedSlot();
    void targetAddressEditedSlot(const QString& value);

    void callsignCheckedSlot();
    void callsignEditedSlot(const QString& value);

public:
    GPSTrailImportTaskWidget(GPSTrailImportTask& task, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~GPSTrailImportTaskWidget();

    void addFile(const std::string& filename);
    void selectFile(const std::string& filename);

    void updateConfig ();
    void updateText ();

    void expertModeChangedSlot();

protected:
    GPSTrailImportTask& task_;

    QHBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    QListWidget* file_list_{nullptr};
    QPushButton* add_file_button_{nullptr};
    QPushButton* delete_file_button_{nullptr};
    QPushButton* delete_all_files_button_{nullptr};

    QTextEdit* text_edit_ {nullptr};

    QLineEdit* sac_edit_ {nullptr};
    QLineEdit* sic_edit_ {nullptr};
    QLineEdit* name_edit_ {nullptr};

    QLineEdit* tod_offset_edit_ {nullptr};

    QCheckBox* set_mode_3a_code_check_ {nullptr};
    QLineEdit* mode_3a_code_edit_ {nullptr};

    QCheckBox* set_target_address_check_ {nullptr};
    QLineEdit* target_address_edit_ {nullptr};

    QCheckBox* set_callsign_check_ {nullptr};
    QLineEdit* callsign_edit_ {nullptr};

    void addMainTab();
    void addConfigTab();
};

#endif // GPSTRAILIMPORTTASKWIDGET_H
