#include "radarplotpositioncalculatortaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "dbobjectcombobox.h"
#include "dbovariableselectionwidget.h"
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
    unsigned int row_cnt=0;

    grid->addWidget (new QLabel ("DBObject"), row_cnt, 0);

    object_box_ = new DBObjectComboBox (false);
    connect (object_box_, SIGNAL(changedObject()), this, SLOT(dbObjectChangedSlot()));
    grid->addWidget (object_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Key Variable"), row_cnt, 0);
    key_box_ = new DBOVariableSelectionWidget ();
    grid->addWidget (key_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Data Source Variable"), row_cnt, 0);
    datasource_box_ = new DBOVariableSelectionWidget ();
    grid->addWidget (datasource_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Range Variable"), row_cnt, 0);
    range_box_ = new DBOVariableSelectionWidget ();
    grid->addWidget (range_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Azimuth Variable"), row_cnt, 0);
    azimuth_box_ = new DBOVariableSelectionWidget ();
    grid->addWidget (azimuth_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Altitude Variable"), row_cnt, 0);
    altitude_box_ = new DBOVariableSelectionWidget ();
    grid->addWidget (altitude_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Latitude Variable"), row_cnt, 0);
    latitude_box_ = new DBOVariableSelectionWidget ();
    grid->addWidget (latitude_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Longitude"), row_cnt, 0);
    longitude_box_ = new DBOVariableSelectionWidget ();
    grid->addWidget (longitude_box_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Number of Plots"), row_cnt, 0);

    count_label_ = new QLabel ("Unknown");
    grid->addWidget (count_label_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Number of Loaded Plots"), row_cnt, 0);

    load_status_label_ = new QLabel ("0");
    grid->addWidget (load_status_label_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Number of Calculated Positions"), row_cnt, 0);

    calculated_status_label_ = new QLabel ("0");
    grid->addWidget (calculated_status_label_, row_cnt, 1);

    row_cnt++;
    grid->addWidget (new QLabel ("Number of Updated Plots"), row_cnt, 0);

    written_status_label_ = new QLabel ("0");
    grid->addWidget (written_status_label_, row_cnt, 1);

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

void RadarPlotPositionCalculatorTaskWidget::dbObjectChangedSlot()
{
    assert (object_box_);

    std::string object_name = object_box_->getObjectName();

    loginf << "RadarPlotPositionCalculatorTaskWidget: dbObjectChangedSlot: " << object_name;

    key_box_->showDBOOnly(object_name);
    datasource_box_->showDBOOnly(object_name);
    range_box_->showDBOOnly(object_name);
    azimuth_box_->showDBOOnly(object_name);
    altitude_box_->showDBOOnly(object_name);

    latitude_box_->showDBOOnly(object_name);
    longitude_box_->showDBOOnly(object_name);
}

void RadarPlotPositionCalculatorTaskWidget::calculateSlot ()
{
    loginf << "RadarPlotPositionCalculatorTaskWidget: calculateSlot";

    assert (!task_.isCalculating());
    task_.calculate();
}


