#ifndef GPSTRAILIMPORTTASKWIDGET_H
#define GPSTRAILIMPORTTASKWIDGET_H

#include <taskwidget.h>

class GPSTrailImportTask;

class QPushButton;
class QListWidget;
class QTextEdit;

class GPSTrailImportTaskWidget : public TaskWidget
{
    Q_OBJECT
  public slots:
    void addFileSlot();
    void deleteFileSlot();
    void deleteAllFilesSlot();
    void selectedFileSlot();
    void updateFileListSlot();

public:
    GPSTrailImportTaskWidget(GPSTrailImportTask& task, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~GPSTrailImportTaskWidget();

    void addFile(const std::string& filename);
    void selectFile(const std::string& filename);

//    void runStarted();
//    void runDone();

    void updateText ();

    void expertModeChangedSlot();

protected:
    GPSTrailImportTask& task_;

    QListWidget* file_list_{nullptr};
    QPushButton* add_file_button_{nullptr};
    QPushButton* delete_file_button_{nullptr};
    QPushButton* delete_all_files_button_{nullptr};

    QTextEdit* text_edit_ {nullptr};
};

#endif // GPSTRAILIMPORTTASKWIDGET_H
