#include "reconstructortaskdialog.h"
#include "reconstructortask.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ReconstructorTaskDialog::ReconstructorTaskDialog(ReconstructorTask& task)
    : QDialog(), task_(task)
{
    setWindowTitle("Reconstruct Reference Trajectories");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

//    task_widget_ = new ReconstructorTaskWidget(task_, this);
//    main_layout->addWidget(task_widget_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &ReconstructorTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &ReconstructorTaskDialog::runClickedSlot);
    button_layout->addWidget(run_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
}

void ReconstructorTaskDialog::updateButtons()
{
    assert (run_button_);

    run_button_->setDisabled(!task_.canRun());

}

void ReconstructorTaskDialog::runClickedSlot()
{
    emit runSignal();
}

void ReconstructorTaskDialog::cancelClickedSlot()
{
    emit cancelSignal();
}

