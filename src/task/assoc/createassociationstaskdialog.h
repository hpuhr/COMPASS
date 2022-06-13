#ifndef CREATEASSOCIATIONSTASKDIALOG_H
#define CREATEASSOCIATIONSTASKDIALOG_H

#include <QDialog>

class CreateAssociationsTask;
class CreateAssociationsTaskWidget;

class QPushButton;

class CreateAssociationsTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void runSignal();
    void cancelSignal();

public slots:
    void runClickedSlot();
    void cancelClickedSlot();

public:
    explicit CreateAssociationsTaskDialog(CreateAssociationsTask& task);

    void updateButtons();

protected:
    CreateAssociationsTask& task_;

    CreateAssociationsTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* run_button_{nullptr};
};

#endif // CREATEASSOCIATIONSTASKDIALOG_H
