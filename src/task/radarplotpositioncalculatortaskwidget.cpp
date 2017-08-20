#include "radarplotpositioncalculatortaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "logger.h"
#include "atsdb.h"
#include "stringconv.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

using namespace Utils::String;


RadarPlotPositionCalculatorTaskWidget::RadarPlotPositionCalculatorTaskWidget(RadarPlotPositionCalculatorTask& task, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), task_(task)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Calculate radar plot positions");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    QGridLayout *grid = new QGridLayout ();

    grid->addWidget (new QLabel ("Number of Plots"), 0, 0);

    count_label_ = new QLabel ("Unknown");
    grid->addWidget (count_label_, 0, 1);

    grid->addWidget (new QLabel ("Number of Loaded Plots"), 1, 0);

    load_status_label_ = new QLabel ("0");
    grid->addWidget (load_status_label_, 1, 1);

    grid->addWidget (new QLabel ("Number of Calculated Positions"), 2, 0);

    calculated_status_label_ = new QLabel ("0");
    grid->addWidget (calculated_status_label_, 2, 1);

    grid->addWidget (new QLabel ("Number of Updated Plots"), 3, 0);

    written_status_label_ = new QLabel ("0");
    grid->addWidget (written_status_label_, 3, 1);

    main_layout->addLayout(grid);

    QPushButton *calc_button = new QPushButton ("Calculate");
    connect(calc_button, SIGNAL( clicked() ), this, SLOT( calculateSlot() ));
    main_layout->addWidget(calc_button);

    main_layout->addStretch();

    setLayout (main_layout);

    show();
}

RadarPlotPositionCalculatorTaskWidget::~RadarPlotPositionCalculatorTaskWidget()
{
}

void RadarPlotPositionCalculatorTaskWidget::update ()
{

}

void RadarPlotPositionCalculatorTaskWidget::calculateSlot ()
{
    loginf << "RadarPlotPositionCalculatorTaskWidget: calculateSlot";

    assert (!task_.isCalculating());
    task_.calculate();
}


