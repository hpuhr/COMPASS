#ifndef ASTERIXIMPORTTASKDIALOG_H
#define ASTERIXIMPORTTASKDIALOG_H

#include <QDialog>

class ASTERIXImportTask;
class ASTERIXImportTaskWidget;

class QPushButton;

class ASTERIXImportTaskDialog : public QDialog
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
    explicit ASTERIXImportTaskDialog(ASTERIXImportTask& task);

    void updateSourcesInfo();
    void updateButtons();

protected:
    ASTERIXImportTask& task_;

    ASTERIXImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
    QPushButton* test_button_{nullptr};
};

#endif // ASTERIXIMPORTTASKDIALOG_H
