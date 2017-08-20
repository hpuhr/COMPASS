#include "radarplotpositioncalculatortaskwidget.h"
#include "logger.h"
#include "atsdb.h"
#include "stringconv.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

using namespace Utils::String;


RadarPlotPositionCalculatorTaskWidget::RadarPlotPositionCalculatorTaskWidget(RadarPlotPositionCalculator& calculator, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), calculator_(calculator)
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

    QLabel *num_label = new QLabel ("# plots");
    grid->addWidget (num_label, 0, 0);

    QLabel *count_label = new QLabel ("UGA"); //intToString(ATSDB::getInstance().count(DBO_PLOTS)).c_str()
    grid->addWidget (count_label, 0, 1);

    QLabel *loaded_label = new QLabel ("# loaded radar plots");
    grid->addWidget (loaded_label, 1, 0);

    load_status_label_ = new QLabel ("0");
    grid->addWidget (load_status_label_, 1, 1);

    QLabel *calc_label = new QLabel ("# calculated plots");
    grid->addWidget (calc_label, 2, 0);

    calculated_status_label_ = new QLabel ("0");
    grid->addWidget (calculated_status_label_, 2, 1);

    QLabel *written_label = new QLabel ("# written plots");
    grid->addWidget (written_label, 3, 0);

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

    //euklid_.calculate();
}


