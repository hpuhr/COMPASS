#include "calculatereferencesstatusdialog.h"
#include "createassociationstask.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "logger.h"
#include "stringconv.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <iomanip>


using namespace std;
using namespace Utils;

CalculateReferencesStatusDialog::CalculateReferencesStatusDialog(
        CalculateReferencesTask& task, QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), task_(task)
{

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setWindowTitle("Calculate References Status");

    setModal(true);

    setMinimumSize(QSize(500, 400));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    int row = 0;
    {
        QGridLayout* general_grid = new QGridLayout();

        ++row;
        general_grid->addWidget(new QLabel("Elapsed Time"), row, 0);
        time_label_ = new QLabel();
        time_label_->setAlignment(Qt::AlignRight);
        general_grid->addWidget(time_label_, row, 1);

        main_layout->addLayout(general_grid);
    }

    main_layout->addStretch();

    // associations

    QLabel* assoc_label = new QLabel("Associations");
    assoc_label->setFont(font_big);
    main_layout->addWidget(assoc_label);

    row = 0;
    {
        QGridLayout* association_grid = new QGridLayout();

        association_grid->addWidget(new QLabel("Status"), row, 0);
        status_label_ = new QLabel();
        status_label_->setAlignment(Qt::AlignRight);
        association_grid->addWidget(status_label_, row, 1);

        main_layout->addLayout(association_grid);
    }

    main_layout->addStretch();

//    dbcont_associated_grid_ = new QGridLayout();
//    updateDBContentAssociatedGrid();
//    main_layout->addLayout(dbcont_associated_grid_);

//    main_layout->addStretch();

    QHBoxLayout* button_layout = new QHBoxLayout();

    ok_button_ = new QPushButton("OK");
    ok_button_->setVisible(false);
    connect(ok_button_, &QPushButton::clicked, this,
            &CalculateReferencesStatusDialog::okClickedSlot);
    button_layout->addWidget(ok_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

void CalculateReferencesStatusDialog::okClickedSlot() { emit closeSignal(); }

void CalculateReferencesStatusDialog::markStartTime()
{
    start_time_ = boost::posix_time::microsec_clock::local_time();

    done_ = false;
}

void CalculateReferencesStatusDialog::setDone()
{
    assert(ok_button_);

    done_ = true;

    updateTime();
    //updateDBContentAssociatedGrid();

    loginf << "CalculateReferencesStatusDialog: setDone: done after " << elapsed_time_str_;

    ok_button_->setVisible(true);
}

void CalculateReferencesStatusDialog::setStatus(const std::string& status)
{
    status_ = status;

    assert(status_label_);
    status_label_->setText(status_.c_str());

    updateTime();
}

//void CalculateReferencesStatusDialog::setAssociationsCounts(
//        std::map<std::string, std::pair<unsigned int,unsigned int>> association_counts)
//{
//    association_counts_ = association_counts;
//    updateDBContentAssociatedGrid();
//}

void CalculateReferencesStatusDialog::updateTime()
{
    assert(time_label_);

    stop_time_ = boost::posix_time::microsec_clock::local_time();
    time_diff_ = stop_time_ - start_time_;
    elapsed_time_str_ =
        String::timeStringFromDouble(time_diff_.total_milliseconds() / 1000.0, false);

    time_label_->setText(elapsed_time_str_.c_str());
}

//void CalculateReferencesStatusDialog::updateDBContentAssociatedGrid()
//{
//    assert(dbcont_associated_grid_);

//    // loginf << "CalculateReferencesStatusDialog: updateDBODoneGrid: rowcount " <<
//    // cat_counters_grid_->rowCount();

//    int row = 1;
//    if (dbcont_associated_grid_->rowCount() == 1)
//    {
//        // loginf << "CalculateReferencesStatusDialog: updateDBODoneGrid: adding first row";

//        QFont font_bold;
//        font_bold.setBold(true);

//        QLabel* dbo_label = new QLabel("DBContent");
//        dbo_label->setFont(font_bold);
//        dbcont_associated_grid_->addWidget(dbo_label, row, 0);

//        QLabel* count_label = new QLabel("Count");
//        count_label->setFont(font_bold);
//        count_label->setAlignment(Qt::AlignRight);
//        dbcont_associated_grid_->addWidget(count_label, row, 1);

//        QLabel* associated_label = new QLabel("Associated");
//        associated_label->setFont(font_bold);
//        associated_label->setAlignment(Qt::AlignRight);
//        dbcont_associated_grid_->addWidget(associated_label, row, 2);

//        QLabel* percent_label = new QLabel("Percent");
//        percent_label->setFont(font_bold);
//        percent_label->setAlignment(Qt::AlignRight);
//        dbcont_associated_grid_->addWidget(percent_label, row, 3);
//    }

//    for (auto& dbo_it : COMPASS::instance().dbContentManager())
//    {
//        ++row;

//        if (dbcont_associated_grid_->rowCount() < row + 1)
//        {
//            // loginf << "CalculateReferencesStatusDialog: updateDBODoneGrid: adding row " <<
//            // row;

//            dbcont_associated_grid_->addWidget(new QLabel(), row, 0);

//            QLabel* count_label = new QLabel();
//            count_label->setAlignment(Qt::AlignRight);
//            dbcont_associated_grid_->addWidget(count_label, row, 1);

//            QLabel* associated_label = new QLabel();
//            associated_label->setAlignment(Qt::AlignRight);
//            dbcont_associated_grid_->addWidget(associated_label, row, 2);

//            QLabel* percent_label = new QLabel();
//            percent_label->setAlignment(Qt::AlignRight);
//            dbcont_associated_grid_->addWidget(percent_label, row, 3);
//        }

//        // loginf << "CalculateReferencesStatusDialog: updateDBODoneGrid: setting row " << row;

//        QLabel* dbo_label =
//            dynamic_cast<QLabel*>(dbcont_associated_grid_->itemAtPosition(row, 0)->widget());
//        assert(dbo_label);
//        dbo_label->setText(dbo_it.first.c_str());

//        size_t count = dbo_it.second->count();
//        size_t assoc_count = 0; //dbo_it.second->associations().size();

//        if (association_counts_.count(dbo_it.first))
//        {
//            count = association_counts_.at(dbo_it.first).first;
//            assoc_count = association_counts_.at(dbo_it.first).second;
//        }

//        QLabel* count_label =
//            dynamic_cast<QLabel*>(dbcont_associated_grid_->itemAtPosition(row, 1)->widget());
//        assert(count_label);
//        count_label->setText(QString::number(count));

//        QLabel* associated_label =
//            dynamic_cast<QLabel*>(dbcont_associated_grid_->itemAtPosition(row, 2)->widget());
//        assert(associated_label);
//        associated_label->setText(QString::number(assoc_count));

//        QLabel* percent_label =
//            dynamic_cast<QLabel*>(dbcont_associated_grid_->itemAtPosition(row, 3)->widget());
//        assert(percent_label);

//        if (count)
//            percent_label->setText(
//                (String::percentToString(100.0 * assoc_count / count) + "%").c_str());
//        else if (count == 0 && assoc_count == 0 && done_)
//            percent_label->setText("100.00%");
//        else
//            percent_label->setText("0%");
//    }
//}
