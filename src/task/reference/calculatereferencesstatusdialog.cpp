#include "calculatereferencesstatusdialog.h"
//#include "createassociationstask.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "compass.h"
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

    setMinimumSize(QSize(500, 550));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(14);

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

    row = 0;
    {
        QGridLayout* status_grid = new QGridLayout();

        status_grid->addWidget(new QLabel("Status"), row, 0);
        status_label_ = new QLabel();
        status_label_->setAlignment(Qt::AlignRight);
        status_grid->addWidget(status_label_, row, 1);

        main_layout->addLayout(status_grid);
    }

    // loaded counts

    main_layout->addSpacing(15);

    QLabel* loaded_label = new QLabel("Loaded Data");
    loaded_label->setFont(font_big);
    main_layout->addWidget(loaded_label);

    dbcont_loaded_grid_ = new QGridLayout();
    main_layout->addLayout(dbcont_loaded_grid_);

    main_layout->addSpacing(15);

    QLabel* used_pos_label = new QLabel("Used Position Data");
    used_pos_label->setFont(font_big);
    main_layout->addWidget(used_pos_label);

    pos_used_grid_ = new QGridLayout();
    main_layout->addLayout(pos_used_grid_);

    // calculated info

    main_layout->addSpacing(15);

    QLabel* calc_info_label = new QLabel("References Info");
    calc_info_label->setFont(font_big);
    main_layout->addWidget(calc_info_label);

    calc_info_grid_ = new QGridLayout();
    main_layout->addLayout(calc_info_grid_);

    result_label_ = new QLabel("");
    result_label_->setVisible(false);
    main_layout->addWidget(result_label_);

    main_layout->addStretch();

    QHBoxLayout* button_layout = new QHBoxLayout();

    ok_button_ = new QPushButton("OK");
    ok_button_->setVisible(false);
    connect(ok_button_, &QPushButton::clicked, this,
            &CalculateReferencesStatusDialog::okClickedSlot);

    QSizePolicy sp_retain = ok_button_->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ok_button_->setSizePolicy(sp_retain);

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

void CalculateReferencesStatusDialog::setDone(const QString& result)
{
    assert(ok_button_);

    done_ = true;

    updateTime();
    //updateDBContentAssociatedGrid();

    loginf << "CalculateReferencesStatusDialog: setDone: done after " << elapsed_time_str_;

    ok_button_->setVisible(true);

    result_label_->setVisible(!result.isEmpty());
    result_label_->setText(result);
}

void CalculateReferencesStatusDialog::setLoadedCountsSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data)
{
    assert(dbcont_loaded_grid_);

    int row = 0;

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* dbo_label = new QLabel("DBContent");
    dbo_label->setFont(font_bold);
    dbcont_loaded_grid_->addWidget(dbo_label, row, 0);

    QLabel* total_count_label = new QLabel("Total Count");
    total_count_label->setFont(font_bold);
    total_count_label->setAlignment(Qt::AlignRight);
    dbcont_loaded_grid_->addWidget(total_count_label, row, 1);

    QLabel* loaded_count_label = new QLabel("Loaded");
    loaded_count_label->setFont(font_bold);
    loaded_count_label->setAlignment(Qt::AlignRight);
    dbcont_loaded_grid_->addWidget(loaded_count_label, row, 2);

    QLabel* percent_label = new QLabel("Percent");
    percent_label->setFont(font_bold);
    percent_label->setAlignment(Qt::AlignRight);
    dbcont_loaded_grid_->addWidget(percent_label, row, 3);

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    for (auto& buf_it : data)
    {
        ++row;

        QLabel* dbcont_label = new QLabel(buf_it.first.c_str());
        dbcont_loaded_grid_->addWidget(dbcont_label, row, 0);

        unsigned int loaded_count = buf_it.second->size();
        unsigned int total_count = dbcont_man.dbContent(buf_it.first).count();

        QLabel* total_count_label = new QLabel();
        total_count_label->setAlignment(Qt::AlignRight);
        total_count_label->setText(QString::number(total_count));
        dbcont_loaded_grid_->addWidget(total_count_label, row, 1);

        float percent = 0;

        if (total_count)
            percent = 100.0 * (float) loaded_count / (float) total_count;

        QLabel* loaded_count_label = new QLabel();
        loaded_count_label->setAlignment(Qt::AlignRight);
        loaded_count_label->setText(QString::number(loaded_count));
        dbcont_loaded_grid_->addWidget(loaded_count_label, row, 2);

        QLabel* percent_label = new QLabel();
        percent_label->setAlignment(Qt::AlignRight);

        if (loaded_count)
            percent_label->setText((String::percentToString(percent) + "%").c_str());
        else
            percent_label->setText("0%");

        dbcont_loaded_grid_->addWidget(percent_label, row, 3);
    }
}

void CalculateReferencesStatusDialog::setUsedPositionCountsSlot (PositionCountsMapStruct counts)
{
    loginf << "CalculateReferencesStatusDialog: setUsedPositionCountsSlot";

    assert(pos_used_grid_);
    //assert (!pos_used_grid_->rowCount());

    int row = 0;

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* dbo_label = new QLabel("DBContent");
    dbo_label->setFont(font_bold);
    pos_used_grid_->addWidget(dbo_label, row, 0);

    QLabel* total_count_label = new QLabel("Total Count");
    total_count_label->setFont(font_bold);
    total_count_label->setAlignment(Qt::AlignRight);
    pos_used_grid_->addWidget(total_count_label, row, 1);

    QLabel* loaded_count_label = new QLabel("Used");
    loaded_count_label->setFont(font_bold);
    loaded_count_label->setAlignment(Qt::AlignRight);
    pos_used_grid_->addWidget(loaded_count_label, row, 2);


    QLabel* percent_label = new QLabel("Percent");
    percent_label->setFont(font_bold);
    percent_label->setAlignment(Qt::AlignRight);
    pos_used_grid_->addWidget(percent_label, row, 3);

    unsigned int used_count, unused_count;

    for (auto& cnt_it : counts.pos_map)
    {
        ++row;

        used_count = cnt_it.second.first;
        unused_count = cnt_it.second.second;

        QLabel* dbcont_label = new QLabel(cnt_it.first.c_str());
        pos_used_grid_->addWidget(dbcont_label, row, 0);

        unsigned int total_count = used_count + unused_count;

        QLabel* total_count_label = new QLabel();
        total_count_label->setAlignment(Qt::AlignRight);
        total_count_label->setText(QString::number(total_count));
        pos_used_grid_->addWidget(total_count_label, row, 1);

        float percent = 0;

        if (total_count)
            percent = 100.0 * (float) used_count / (float) total_count;

        QLabel* loaded_count_label = new QLabel();
        loaded_count_label->setAlignment(Qt::AlignRight);
        loaded_count_label->setText(QString::number(used_count));
        pos_used_grid_->addWidget(loaded_count_label, row, 2);

        QLabel* percent_label = new QLabel();
        percent_label->setAlignment(Qt::AlignRight);

        if (used_count)
            percent_label->setText((String::percentToString(percent) + "%").c_str());
        else
            percent_label->setText("0%");

        pos_used_grid_->addWidget(percent_label, row, 3);
    }

}

void CalculateReferencesStatusDialog::setCalculateInfoSlot(CalcInfoVectorStruct info)
{
    loginf << "CalculateReferencesStatusDialog: setCalculateInfoSlot";

    assert(calc_info_grid_);

    int row = 0;

    string text1, text2;

    for (auto& txt_it : info.info_vec)
    {
        ++row;

        text1 = txt_it.first;
        text2 = txt_it.second;

        calc_info_grid_->addWidget(new QLabel(text1.c_str()), row, 0);

        QLabel* text2_label = new QLabel(text2.c_str());
        text2_label->setAlignment(Qt::AlignRight);
        calc_info_grid_->addWidget(text2_label, row, 1);
    }
}

void CalculateReferencesStatusDialog::setStatusSlot(const QString& status)
{
    status_ = status.toStdString();

    assert(status_label_);
    status_label_->setText(status);

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
//    assert(dbcont_loaded_grid_);

//    // loginf << "CalculateReferencesStatusDialog: setLoadedCounts: rowcount " <<
//    // cat_counters_grid_->rowCount();

//    int row = 1;
//    if (dbcont_loaded_grid_->rowCount() == 1)
//    {
//        // loginf << "CalculateReferencesStatusDialog: setLoadedCounts: adding first row";

//        QFont font_bold;
//        font_bold.setBold(true);

//        QLabel* dbo_label = new QLabel("DBContent");
//        dbo_label->setFont(font_bold);
//        dbcont_loaded_grid_->addWidget(dbo_label, row, 0);

//        QLabel* count_label = new QLabel("Count");
//        count_label->setFont(font_bold);
//        count_label->setAlignment(Qt::AlignRight);
//        dbcont_loaded_grid_->addWidget(count_label, row, 1);

//        QLabel* associated_label = new QLabel("Associated");
//        associated_label->setFont(font_bold);
//        associated_label->setAlignment(Qt::AlignRight);
//        dbcont_loaded_grid_->addWidget(associated_label, row, 2);

//        QLabel* percent_label = new QLabel("Percent");
//        percent_label->setFont(font_bold);
//        percent_label->setAlignment(Qt::AlignRight);
//        dbcont_loaded_grid_->addWidget(percent_label, row, 3);
//    }

//    for (auto& dbo_it : COMPASS::instance().dbContentManager())
//    {
//        ++row;

//        if (dbcont_loaded_grid_->rowCount() < row + 1)
//        {
//            // loginf << "CalculateReferencesStatusDialog: setLoadedCounts: adding row " <<
//            // row;

//            dbcont_loaded_grid_->addWidget(new QLabel(), row, 0);

//            QLabel* count_label = new QLabel();
//            count_label->setAlignment(Qt::AlignRight);
//            dbcont_loaded_grid_->addWidget(count_label, row, 1);

//            QLabel* associated_label = new QLabel();
//            associated_label->setAlignment(Qt::AlignRight);
//            dbcont_loaded_grid_->addWidget(associated_label, row, 2);

//            QLabel* percent_label = new QLabel();
//            percent_label->setAlignment(Qt::AlignRight);
//            dbcont_loaded_grid_->addWidget(percent_label, row, 3);
//        }

//        // loginf << "CalculateReferencesStatusDialog: setLoadedCounts: setting row " << row;

//        QLabel* dbo_label =
//            dynamic_cast<QLabel*>(dbcont_loaded_grid_->itemAtPosition(row, 0)->widget());
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
//            dynamic_cast<QLabel*>(dbcont_loaded_grid_->itemAtPosition(row, 1)->widget());
//        assert(count_label);
//        count_label->setText(QString::number(count));

//        QLabel* associated_label =
//            dynamic_cast<QLabel*>(dbcont_loaded_grid_->itemAtPosition(row, 2)->widget());
//        assert(associated_label);
//        associated_label->setText(QString::number(assoc_count));

//        QLabel* percent_label =
//            dynamic_cast<QLabel*>(dbcont_loaded_grid_->itemAtPosition(row, 3)->widget());
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
