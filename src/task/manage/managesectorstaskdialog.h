#pragma once

#include <QDialog>

class ManageSectorsTask;
class ManageSectorsTaskWidget;

class ManageSectorsTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void doneSignal();

public slots:
    void doneClickedSlot();

public:
    ManageSectorsTaskDialog(ManageSectorsTask& task);

    void updateFileList();
    void updateParseMessage();

protected:
    ManageSectorsTask& task_;

    ManageSectorsTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* done_button_{nullptr};
};

