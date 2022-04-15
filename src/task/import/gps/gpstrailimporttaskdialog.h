#ifndef GPSTRAILIMPORTTASKDIALOG_H
#define GPSTRAILIMPORTTASKDIALOG_H

#include <QDialog>

class GPSTrailImportTask;
class GPSTrailImportTaskWidget;

class GPSTrailImportTaskDialog: public QDialog
{
    Q_OBJECT

signals:
    void doneSignal();
    void importSignal();

public slots:
    void importClickedSlot();
    void doneClickedSlot();

public:
    GPSTrailImportTaskDialog(GPSTrailImportTask& task);

    void updateButtons();

protected:
    GPSTrailImportTask& task_;

    GPSTrailImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* import_button_{nullptr};
    QPushButton* done_button_{nullptr};
};

#endif // GPSTRAILIMPORTTASKDIALOG_H
