#include "createartasassociationstaskwidget.h"
#include "createartasassociationstask.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

CreateARTASAssociationsTaskWidget::CreateARTASAssociationsTaskWidget(CreateARTASAssociationsTask& task, QWidget* parent,
                                                                     Qt::WindowFlags f)
    : QWidget (parent, f), task_(task)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Create ARTAS Associations");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    calc_button_ = new QPushButton ("Calculate");
    connect(calc_button_, SIGNAL( clicked() ), this, SLOT( runSlot() ));
    main_layout->addWidget(calc_button_);


    setLayout (main_layout);

    //update();

    show();
}

CreateARTASAssociationsTaskWidget::~CreateARTASAssociationsTaskWidget()
{
}

void CreateARTASAssociationsTaskWidget::runSlot ()
{
    loginf << "CreateARTASAssociationsTaskWidget: calculateSlot";

//    if (!task_.canCalculate())
//    {
//        QMessageBox::warning (this, "Unable to Calculate",
//                              "The task can not be peformed with the entered items.\n"
//                              "The following conditions have to be met: The DBObject must exist, must have data, and"
//                              " all variables have to be set and exist in the current schema and database");
//        return;
//    }

    assert (calc_button_);

//    std::string db_object_str = task_.dbObjectStr();
//    DBObjectManager& obj_man = ATSDB::instance().objectManager();

//    assert (obj_man.existsObject(db_object_str));
//    DBObject& db_object = obj_man.object(db_object_str);

//    bool not_final = false;
//    for (auto ds_it = db_object.dsBegin(); ds_it != db_object.dsEnd(); ++ds_it)
//    {
//        ds_it->second.finalize();
//        if (!ds_it->second.isFinalized())
//        {
//            not_final = true;
//            break;
//        }
//    }

//    if (not_final)
//    {
//        QMessageBox::warning (this, "EPSG Value Wrong",
//                              "The coordinates of the data sources of selected database object could not be calculated."
//                              " Please select a suitable EPSG value and try again");
//        return;
//    }

    loginf << "CreateARTASAssociationsTaskWidget: calculateSlot: starting run";

    calc_button_->setDisabled(true);

    //assert (!task_.isCalculating());
    task_.run();
}

void CreateARTASAssociationsTaskWidget::runDoneSlot ()
{
    assert (calc_button_);
    calc_button_->setDisabled(false);
}
