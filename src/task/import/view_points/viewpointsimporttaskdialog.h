#ifndef VIEWPOINTSIMPORTTASKDIALOG_H
#define VIEWPOINTSIMPORTTASKDIALOG_H

#include <QDialog>

class ViewPointsImportTask;
class ViewPointsImportTaskWidget;

class QPushButton;

class ViewPointsImportTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void importSignal();
    void cancelSignal();

public slots:
    void importClickedSlot();
    void cancelClickedSlot();

public:
    ViewPointsImportTaskDialog(ViewPointsImportTask& task);

    void updateContext ();
    void updateButtons();

protected:
    ViewPointsImportTask& task_;

    ViewPointsImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
};

#endif // VIEWPOINTSIMPORTTASKDIALOG_H
