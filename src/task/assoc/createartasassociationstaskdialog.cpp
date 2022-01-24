#include "createartasassociationstaskdialog.h"
#include "createartasassociationstask.h"
#include "createartasassociationstaskwidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

CreateARTASAssociationsTaskDialog::CreateARTASAssociationsTaskDialog(CreateARTASAssociationsTask& task)
: QDialog(), task_(task)
{
    setWindowTitle("Calculate Associations from ARTAS");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    task_widget_ = new CreateARTASAssociationsTaskWidget(task_, this);
    main_layout->addWidget(task_widget_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &CreateARTASAssociationsTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &CreateARTASAssociationsTaskDialog::runClickedSlot);
    button_layout->addWidget(run_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
}

void CreateARTASAssociationsTaskDialog::updateButtons()
{
    assert (run_button_);

    run_button_->setDisabled(!task_.canRun());

}

void CreateARTASAssociationsTaskDialog::runClickedSlot()
{
    emit runSignal();
}

void CreateARTASAssociationsTaskDialog::cancelClickedSlot()
{
    emit cancelSignal();
}

