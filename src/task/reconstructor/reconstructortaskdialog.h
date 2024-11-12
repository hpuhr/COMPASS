#pragma once

#include <QDialog>

class QPushButton;
class QStackedWidget;
class QComboBox;

class ReconstructorTask;

class ReconstructorTaskDialog : public QDialog
{
    Q_OBJECT
    
public slots:
    void reconstructorMethodChangedSlot(const QString& value);

public:
    ReconstructorTaskDialog(ReconstructorTask& task);
    virtual ~ReconstructorTaskDialog();

    void showCurrentReconstructorWidget();
    void updateButtons();

protected:
    ReconstructorTask& task_;

    // order in stack has to be the same as in box
    QComboBox*      reconstructor_box_  {nullptr};
    QStackedWidget* reconstructor_widget_stack_ {nullptr};

    QPushButton* cancel_button_{nullptr};
    QPushButton* run_button_{nullptr};
};

