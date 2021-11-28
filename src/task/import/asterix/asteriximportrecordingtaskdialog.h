#ifndef ASTERIXIMPORTRECORDINGTASKDIALOG_H
#define ASTERIXIMPORTRECORDINGTASKDIALOG_H

#include <QDialog>

class ASTERIXImportTask;
class ASTERIXImportTaskWidget;

class QPushButton;

class ASTERIXImportRecordingTaskDialog : public QDialog
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
    explicit ASTERIXImportRecordingTaskDialog(ASTERIXImportTask& task);

    void updateButtons();

protected:
    ASTERIXImportTask& task_;

    ASTERIXImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
    QPushButton* test_button_{nullptr};
};

#endif // ASTERIXIMPORTRECORDINGTASKDIALOG_H
