#include "asterixstatusdialog.h"
#include "logger.h"
#include "stringconv.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include <iomanip>

using namespace std;
using namespace Utils;

ASTERIXStatusDialog::ASTERIXStatusDialog(const std::string& filename, bool test, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f), filename_(filename), test_(test)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setWindowTitle ("Import ASTERIX Data Status");
    setModal(true);

    setMinimumSize(QSize(600, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    int row = 0;
    {
        QGridLayout* general_grid = new QGridLayout();

        general_grid->addWidget(new QLabel("Filename"), row, 0);
        QLabel* filename_label = new QLabel(filename_.c_str());
        filename_label->setAlignment(Qt::AlignRight);
        general_grid->addWidget(filename_label, row, 1);

        ++row;
        general_grid->addWidget(new QLabel("Elapsed Time"), row, 0);
        time_label_ = new QLabel ();
        time_label_->setAlignment(Qt::AlignRight);
        general_grid->addWidget(time_label_, row, 1);

        main_layout->addLayout(general_grid);
    }

    main_layout->addStretch();

    QLabel* read_label = new QLabel("File Reading Status");
    read_label->setFont(font_big);
    main_layout->addWidget(read_label);

    row = 0;
    {
        QGridLayout* count_grid = new QGridLayout();

        count_grid->addWidget(new QLabel("Frames Read"), row, 0);
        num_frames_label_ = new QLabel ();
        num_frames_label_->setAlignment(Qt::AlignRight);
        count_grid->addWidget(num_frames_label_, row, 1);

        ++row;
        count_grid->addWidget(new QLabel("Records Read"), row, 0);
        num_records_label_ = new QLabel ();
        num_records_label_->setAlignment(Qt::AlignRight);
        count_grid->addWidget(num_records_label_, row, 1);

        ++row;
        count_grid->addWidget(new QLabel("Decoding Errors"), row, 0);
        num_errors_label_ = new QLabel ();
        num_errors_label_->setAlignment(Qt::AlignRight);
        count_grid->addWidget(num_errors_label_, row, 1);

        ++row;
        count_grid->addWidget(new QLabel("Records Read Rate"), row, 0);
        num_records_rate_label_ = new QLabel ();
        num_records_rate_label_->setAlignment(Qt::AlignRight);
        count_grid->addWidget(num_records_rate_label_, row, 1);

        main_layout->addLayout(count_grid);
    }

    main_layout->addStretch();

    QLabel* map_label = new QLabel("Mapping Status");
    map_label->setFont(font_big);
    main_layout->addWidget(map_label);

    row = 0;
    {
        QGridLayout* count_grid = new QGridLayout();

        count_grid->addWidget(new QLabel("Records Mapped"), row, 0);
        records_mapped_label_ = new QLabel ();
        records_mapped_label_->setAlignment(Qt::AlignRight);
        count_grid->addWidget(records_mapped_label_, row, 1);

        ++row;
        count_grid->addWidget(new QLabel("Records Not Mapped"), row, 0);
        records_not_mapped_label_ = new QLabel ();
        records_not_mapped_label_->setAlignment(Qt::AlignRight);
        count_grid->addWidget(records_not_mapped_label_, row, 1);

        main_layout->addLayout(count_grid);
    }

    cat_counters_grid_ = new QGridLayout();
    main_layout->addLayout(cat_counters_grid_);

    main_layout->addStretch();

    QLabel* db_label = new QLabel("DB Insert Status");
    db_label->setFont(font_big);
    main_layout->addWidget(db_label);

    row = 0;
    {
        QGridLayout* count2_grid = new QGridLayout();
        count2_grid->addWidget(new QLabel("Records Created"), row, 0);
        records_created_label_ = new QLabel ();
        records_created_label_->setAlignment(Qt::AlignRight);
        count2_grid->addWidget(records_created_label_, row, 1);

        ++row;
        count2_grid->addWidget(new QLabel("Records Inserted"), row, 0);
        records_inserted_label_ = new QLabel ();
        records_inserted_label_->setAlignment(Qt::AlignRight);
        count2_grid->addWidget(records_inserted_label_, row, 1);

        ++row;
        count2_grid->addWidget(new QLabel("Records Inserted Rate"), row, 0);
        records_inserted_rate_label_ = new QLabel ();
        records_inserted_rate_label_->setAlignment(Qt::AlignRight);
        count2_grid->addWidget(records_inserted_rate_label_, row, 1);

        main_layout->addLayout(count2_grid);
    }

    dbo_counters_grid_ = new QGridLayout();
    main_layout->addLayout(dbo_counters_grid_);

    main_layout->addStretch();

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    ok_button_ = new QPushButton ("OK");
    ok_button_->setVisible(false);
    connect(ok_button_, &QPushButton::clicked, this, &ASTERIXStatusDialog::okClickedSlot);
    button_layout->addWidget(ok_button_);

    main_layout->addLayout(button_layout);

    setLayout (main_layout);
}

void ASTERIXStatusDialog::markStartTime ()
{
    start_time_ = boost::posix_time::microsec_clock::local_time();
}

void ASTERIXStatusDialog::okClickedSlot()
{
    emit closeSignal();
}

void ASTERIXStatusDialog::setDone ()
{
    assert (ok_button_);

    updateTime ();

    loginf << "ASTERIXStatusWidget: setDone: done after " << elapsed_time_str_;

    ok_button_->setVisible(true);
}

void ASTERIXStatusDialog::numFrames (unsigned int cnt)
{
    assert (num_frames_label_);

    num_frames_ = cnt;
    num_frames_label_->setText(QString::number(num_frames_));

    updateTime();
}
void ASTERIXStatusDialog::numRecords (unsigned int cnt)
{
    assert (num_records_label_);
    assert (num_records_rate_label_);

    num_records_ = cnt;
    num_records_label_->setText(QString::number(num_records_));

    updateTime();

    double records_per_second = num_records_/(time_diff_.total_milliseconds()/1000.0);
    std::string records_rate_str_ = std::to_string(static_cast<int>(records_per_second))+" (e/s)";
    num_records_rate_label_->setText(records_rate_str_.c_str());
}

void ASTERIXStatusDialog::numErrors (unsigned int cnt)
{
    logdbg << "ASTERIXStatusDialog: addNumErrors: " << cnt;

    assert (num_errors_label_);

    num_errors_ = cnt;
    num_errors_label_->setText(QString::number(num_errors_));

    updateTime();
}

void ASTERIXStatusDialog::addNumMapped (unsigned int cnt)
{
    assert (records_mapped_label_);

    records_mapped_ += cnt;
    records_mapped_label_->setText(QString::number(records_mapped_));

    updateTime();
}
void ASTERIXStatusDialog::addNumNotMapped (unsigned int cnt)
{
    assert (records_not_mapped_label_);

    records_not_mapped_ += cnt;
    records_not_mapped_label_->setText(QString::number(records_not_mapped_));

    updateTime();
}
void ASTERIXStatusDialog::addNumCreated (unsigned int cnt)
{
    assert (records_created_label_);

    records_created_ += cnt;
    records_created_label_->setText(QString::number(records_created_));

    updateTime();
}
void ASTERIXStatusDialog::addNumInserted (const std::string& dbo_name, unsigned int cnt)
{
    assert (records_inserted_label_);
    assert (records_inserted_rate_label_);

    records_inserted_ += cnt;
    dbo_inserted_counts_[dbo_name] += cnt;
    records_inserted_label_->setText(QString::number(records_inserted_));

    updateDBObjectGrid();
    updateTime();

    double records_per_second = records_inserted_/(time_diff_.total_milliseconds()/1000.0);
    std::string records_rate_str_ = std::to_string(static_cast<int>(records_per_second))+" (e/s)";
    records_inserted_rate_label_->setText(records_rate_str_.c_str());
}

void ASTERIXStatusDialog::setCategoryCounts (const std::map<unsigned int, size_t>& counts)
{
    category_read_counts_ = counts;

    updateCategoryGrid();
}
void ASTERIXStatusDialog::addMappedCounts (const std::map<unsigned int, std::pair<size_t,size_t>>& counts)
{
    for (auto& mapped_cnt_it : counts)
    {
        category_mapped_counts_[mapped_cnt_it.first].first += mapped_cnt_it.second.first;
        category_mapped_counts_[mapped_cnt_it.first].second += mapped_cnt_it.second.second;
    }

    updateCategoryGrid();
}

void ASTERIXStatusDialog::updateTime ()
{
    assert (time_label_);

    stop_time_ = boost::posix_time::microsec_clock::local_time();
    time_diff_ = stop_time_ - start_time_;
    elapsed_time_str_ = String::timeStringFromDouble(time_diff_.total_milliseconds()/1000.0, false);

    time_label_->setText(elapsed_time_str_.c_str());
}

void ASTERIXStatusDialog::updateCategoryGrid ()
{
    assert (cat_counters_grid_);

    //loginf << "ASTERIXStatusDialog: updateCategoryGrid: rowcount " << cat_counters_grid_->rowCount();

    int row = 1;
    if (cat_counters_grid_->rowCount() == 1)
    {
        //loginf << "ASTERIXStatusDialog: updateCategoryGrid: adding first row";

        QFont font_bold;
        font_bold.setBold(true);

        QLabel* cat_label = new QLabel("Category");
        cat_label->setFont(font_bold);
        cat_counters_grid_->addWidget(cat_label, row, 0);

        QLabel* count_label = new QLabel("Count");
        count_label->setFont(font_bold);
        count_label->setAlignment(Qt::AlignRight);
        cat_counters_grid_->addWidget(count_label, row, 1);

        QLabel* mapped_label = new QLabel("Mapped");
        mapped_label->setFont(font_bold);
        mapped_label->setAlignment(Qt::AlignRight);
        cat_counters_grid_->addWidget(mapped_label, row, 2);

        QLabel* not_mapped_label = new QLabel("Not Mapped");
        not_mapped_label->setFont(font_bold);
        not_mapped_label->setAlignment(Qt::AlignRight);
        cat_counters_grid_->addWidget(not_mapped_label, row, 3);
    }

    for (auto& cat_cnt_it : category_read_counts_)
    {
        ++row;

        if (cat_counters_grid_->rowCount() < row+1)
        {
            //loginf << "ASTERIXStatusDialog: updateCategoryGrid: adding row " << row;

            cat_counters_grid_->addWidget(new QLabel(), row, 0);

            QLabel* count_label = new QLabel();
            count_label->setAlignment(Qt::AlignRight);
            cat_counters_grid_->addWidget(count_label, row, 1);

            QLabel* mapped_label = new QLabel();
            mapped_label->setAlignment(Qt::AlignRight);
            cat_counters_grid_->addWidget(mapped_label, row, 2);

            QLabel* not_mapped_label = new QLabel();
            not_mapped_label->setAlignment(Qt::AlignRight);
            cat_counters_grid_->addWidget(not_mapped_label, row, 3);
        }

        //loginf << "ASTERIXStatusDialog: updateCategoryGrid: setting row " << row;

        QLabel* cat_label = dynamic_cast<QLabel*>(cat_counters_grid_->itemAtPosition(row, 0)->widget());
        assert (cat_label);
        cat_label->setText(String::categoryString(cat_cnt_it.first).c_str());

        QLabel* count_label = dynamic_cast<QLabel*>(cat_counters_grid_->itemAtPosition(row, 1)->widget());
        assert (count_label);
        count_label->setText(std::to_string(cat_cnt_it.second).c_str());

        QLabel* mapped_label = dynamic_cast<QLabel*>(cat_counters_grid_->itemAtPosition(row, 2)->widget());
        assert (mapped_label);
        mapped_label->setText(std::to_string(category_mapped_counts_[cat_cnt_it.first].first).c_str());

        QLabel* not_mapped_label = dynamic_cast<QLabel*>(cat_counters_grid_->itemAtPosition(row, 3)->widget());
        assert (not_mapped_label);
        not_mapped_label->setText(std::to_string(category_mapped_counts_[cat_cnt_it.first].second).c_str());
    }
}

void ASTERIXStatusDialog::updateDBObjectGrid ()
{
    assert (dbo_counters_grid_);

    int row = 1;
    if (dbo_counters_grid_->rowCount() == 1)
    {
        QFont font_bold;
        font_bold.setBold(true);

        QLabel* dbo_label = new QLabel("DBObject");
        dbo_label->setFont(font_bold);
        dbo_counters_grid_->addWidget(dbo_label, row, 0);

        QLabel* count_label = new QLabel("Count");
        count_label->setFont(font_bold);
        count_label->setAlignment(Qt::AlignRight);
        dbo_counters_grid_->addWidget(count_label, row, 1);
    }

    for (auto& dbo_cnt_it : dbo_inserted_counts_)
    {
        ++row;

        if (dbo_counters_grid_->rowCount() < row+1)
        {
            dbo_counters_grid_->addWidget(new QLabel(), row, 0);

            QLabel* count_label = new QLabel();
            count_label->setAlignment(Qt::AlignRight);
            dbo_counters_grid_->addWidget(count_label, row, 1);
        }

        QLabel* dbo_label = dynamic_cast<QLabel*>(dbo_counters_grid_->itemAtPosition(row, 0)->widget());
        assert (dbo_label);
        dbo_label->setText(dbo_cnt_it.first.c_str());

        QLabel* count_label = dynamic_cast<QLabel*>(dbo_counters_grid_->itemAtPosition(row, 1)->widget());
        assert (count_label);
        count_label->setText(std::to_string(dbo_cnt_it.second).c_str());
    }
}


