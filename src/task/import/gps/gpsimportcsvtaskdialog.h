#pragma once

#include <QDialog>

class GPSImportCSVTask;
class GPSImportCSVTaskWidget;

class GPSImportCSVTaskDialog: public QDialog
{
    Q_OBJECT

signals:
    void cancelSignal();
    void importSignal();

public slots:
    void cancelClickedSlot();
    void importClickedSlot();

public:
    GPSImportCSVTaskDialog(GPSImportCSVTask& task);

    void updateButtons();

protected:
    GPSImportCSVTask& task_;

    GPSImportCSVTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
};
