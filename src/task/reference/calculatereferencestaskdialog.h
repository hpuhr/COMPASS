#ifndef CALCULATEREFERENCESTASKDIALOG_H
#define CALCULATEREFERENCESTASKDIALOG_H


#include <QDialog>


class CalculateReferencesTask;

class QPushButton;

class CalculateReferencesTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void runSignal();
    void cancelSignal();

public slots:
    void runClickedSlot();
    void cancelClickedSlot();

public:
    CalculateReferencesTaskDialog(CalculateReferencesTask& task);

    void updateButtons();

protected:
    CalculateReferencesTask& task_;

    QPushButton* cancel_button_{nullptr};
    QPushButton* run_button_{nullptr};
};

#endif // CALCULATEREFERENCESTASKDIALOG_H
