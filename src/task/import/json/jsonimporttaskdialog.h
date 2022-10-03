#ifndef JSONIMPORTTASKDIALOG_H
#define JSONIMPORTTASKDIALOG_H


#include <QDialog>

class JSONImportTask;
class JSONImportTaskWidget;

class QPushButton;

class JSONImportTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void testTmportSignal();
    void importSignal();
    void cancelSignal();

public slots:
    void testImportClickedSlot();
    void importClickedSlot();
    void cancelClickedSlot();

public:
    explicit JSONImportTaskDialog(JSONImportTask& task);

    void updateSource();
    void updateButtons();

protected:
    JSONImportTask& task_;

    JSONImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
    QPushButton* test_button_{nullptr};
};

#endif // JSONIMPORTTASKDIALOG_H
