#include "radarplotpositioncalculatortaskdialog.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"

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

    RadarPlotPositionCalculatorTaskWidget* widget = new RadarPlotPositionCalculatorTaskWidget(task_);
    main_layout->addWidget(widget);


    main_layout->addStretch();

    QHBoxLayout* button_layout = new QHBoxLayout();

    QPushButton* cancel_button = new QPushButton("Cancel");
    connect(cancel_button, &QPushButton::clicked,
            this, &RadarPlotPositionCalculatorTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button);

    button_layout->addStretch();

    QPushButton* ok_button = new QPushButton("OK");
    connect(ok_button, &QPushButton::clicked,
            this, &RadarPlotPositionCalculatorTaskDialog::okClickedSlot);
    button_layout->addWidget(ok_button);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

bool RadarPlotPositionCalculatorTaskDialog::runWanted() const
{
    return run_wanted_;
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
