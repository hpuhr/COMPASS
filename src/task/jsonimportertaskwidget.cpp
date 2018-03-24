#include "jsonimportertaskwidget.h"
#include "jsonimportertask.h"
#include "dbobjectcombobox.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"
#include "atsdb.h"
#include "stringconv.h"
#include "dbobjectmanager.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

JSONImporterTaskWidget::JSONImporterTaskWidget(JSONImporterTask& task, QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f), task_(task)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Import JSON data");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    QGridLayout *grid = new QGridLayout ();
    unsigned int row_cnt=0;

    grid->addWidget (new QLabel ("DBObject"), row_cnt, 0);

    object_box_ = new DBObjectComboBox (false);
    connect (object_box_, SIGNAL(changedObject()), this, SLOT(dbObjectChangedSlot()));
    grid->addWidget (object_box_, row_cnt, 1);

    // TODO

    main_layout->addLayout(grid);

    test_button_ = new QPushButton ("Test Import");
    connect(test_button_, SIGNAL(clicked()), this, SLOT(testImportSlot()));
    main_layout->addWidget(test_button_);

    import_button_ = new QPushButton ("Import");
    connect(import_button_, SIGNAL(clicked()), this, SLOT(importSlot()));
    main_layout->addWidget(import_button_);

    setLayout (main_layout);

    update();

    show();
}

JSONImporterTaskWidget::~JSONImporterTaskWidget()
{

}

void JSONImporterTaskWidget::update ()
{

}

void JSONImporterTaskWidget::testImport()
{

}

void JSONImporterTaskWidget::import()
{

}

void JSONImporterTaskWidget::dbObjectChangedSlot()
{
    assert (object_box_);

    std::string object_name = object_box_->getObjectName();

    loginf << "RadarPlotPositionCalculatorTaskWidget: dbObjectChangedSlot: " << object_name;
    setDBOBject (object_name);
}

void JSONImporterTaskWidget::testImportSlot ()
{

}

void JSONImporterTaskWidget::importSlot ()
{

}

void JSONImporterTaskWidget::testImportDoneSlot ()
{

}

void JSONImporterTaskWidget::importDoneSlot ()
{

}

void JSONImporterTaskWidget::setDBOBject (const std::string& object_name)
{
    task_.dbObjectStr(object_name);

//    key_box_->showDBOOnly(object_name);
//    datasource_box_->showDBOOnly(object_name);
//    range_box_->showDBOOnly(object_name);
//    azimuth_box_->showDBOOnly(object_name);
//    altitude_box_->showDBOOnly(object_name);

//    latitude_box_->showDBOOnly(object_name);
//    longitude_box_->showDBOOnly(object_name);
}
