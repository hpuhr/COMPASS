#pragma once

#include <QDialog>

class GPSTrailImportTask;
class GPSTrailImportTaskWidget;

class GPSTrailImportTaskDialog: public QDialog
{
    Q_OBJECT

signals:
    void cancelSignal();
    void importSignal();

public slots:
    void cancelClickedSlot();
    void importClickedSlot();

public:
    GPSTrailImportTaskDialog(GPSTrailImportTask& task);

    void updateButtons();

protected:
    GPSTrailImportTask& task_;

    GPSTrailImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
};
