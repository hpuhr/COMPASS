#include "radarplotpositioncalculatortaskdialog.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

RadarPlotPositionCalculatorTaskDialog::RadarPlotPositionCalculatorTaskDialog(RadarPlotPositionCalculatorTask& task)
    : task_(task)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setWindowTitle("Calculate Radar Plot Positions");

    setModal(true);

    setMinimumSize(QSize(300, 200));

    QVBoxLayout* main_layout = new QVBoxLayout();

    widget_.reset(new RadarPlotPositionCalculatorTaskWidget(task_));
    main_layout->addWidget(widget_.get());

    main_layout->addStretch();

    QHBoxLayout* button_layout = new QHBoxLayout();

    QPushButton* cancel_button = new QPushButton("Cancel");
    connect(cancel_button, &QPushButton::clicked,
            this, &RadarPlotPositionCalculatorTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button);

    button_layout->addStretch();

    ok_button_ = new QPushButton("OK");
    connect(ok_button_, &QPushButton::clicked,
            this, &RadarPlotPositionCalculatorTaskDialog::okClickedSlot);
    button_layout->addWidget(ok_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

RadarPlotPositionCalculatorTaskDialog::~RadarPlotPositionCalculatorTaskDialog()
{
    loginf << "RadarPlotPositionCalculatorTaskDialog: dtor";
    widget_ = nullptr;
}

bool RadarPlotPositionCalculatorTaskDialog::runWanted() const
{
    return run_wanted_;
}

void RadarPlotPositionCalculatorTaskDialog::updateCanRun()
{
    ok_button_->setEnabled(task_.canRun());
}

void RadarPlotPositionCalculatorTaskDialog::okClickedSlot()
{
    run_wanted_ = true;

    emit closeSignal();
}
void RadarPlotPositionCalculatorTaskDialog::cancelClickedSlot()
{
    run_wanted_ = false;

    emit closeSignal();
}
