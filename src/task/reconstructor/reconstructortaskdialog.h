#pragma once

#include <QDialog>

class QPushButton;

class ReconstructorTask;

class ReconstructorTaskDialog : public QDialog
{
    Q_OBJECT

  signals:
    void runSignal();
    void cancelSignal();

  public slots:
    void runClickedSlot();
    void cancelClickedSlot();

  public:
    ReconstructorTaskDialog(ReconstructorTask& task);

    void updateButtons();

  protected:
    ReconstructorTask& task_;

    QPushButton* cancel_button_{nullptr};
    QPushButton* run_button_{nullptr};
};

