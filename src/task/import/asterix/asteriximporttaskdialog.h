#ifndef ASTERIXIMPORTTASKDIALOG_H
#define ASTERIXIMPORTTASKDIALOG_H

#include <QDialog>

class ASTERIXImportTask;
class ASTERIXImportTaskWidget;

class QPushButton;

class ASTERIXImportTaskDialog : public QDialog
{
    Q_OBJECT

public slots:
    void importClickedSlot();
    void cancelClickedSlot();

public:
    explicit ASTERIXImportTaskDialog(ASTERIXImportTask& task, QWidget* parent = nullptr);

    void updateSourcesInfo();
    void updateButtons();
    void updateTitle();

protected:
    void configChanged();
    void decodingStateChangedSlot();

    ASTERIXImportTask& task_;

    ASTERIXImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
};

#endif // ASTERIXIMPORTTASKDIALOG_H
