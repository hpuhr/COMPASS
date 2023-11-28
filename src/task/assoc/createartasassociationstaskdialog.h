#ifndef CREATEARTASASSOCIATIONSTASKDIALOG_H
#define CREATEARTASASSOCIATIONSTASKDIALOG_H

#include <QDialog>

class CreateARTASAssociationsTask;
class CreateARTASAssociationsTaskWidget;

class QPushButton;

class CreateARTASAssociationsTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void runSignal();
    void cancelSignal();

public slots:
    void runClickedSlot();
    void cancelClickedSlot();

public:
    explicit CreateARTASAssociationsTaskDialog(CreateARTASAssociationsTask& task);

    void updateButtons();

protected:
    CreateARTASAssociationsTask& task_;

    CreateARTASAssociationsTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* run_button_{nullptr};
};

#endif // CREATEARTASASSOCIATIONSTASKDIALOG_H
