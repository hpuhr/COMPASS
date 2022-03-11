#include "dbdatasourcewidget.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "stringconv.h"

#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

using namespace std;
using namespace Utils;

namespace dbContent
{


DBDataSourceWidget::DBDataSourceWidget(DBDataSource& src, QWidget *parent)
    : QWidget(parent), src_(src), ds_man_(COMPASS::instance().dataSourceManager())
{
    main_layout_ = new QVBoxLayout();

    grid_layout_ = new QGridLayout();

    updateContent();

    main_layout_->addLayout(grid_layout_);

    setLayout(main_layout_);

    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void DBDataSourceWidget::updateContent()
{
    if (needsRecreate())
        recreateWidgets();

    updateWidgets();
}

bool DBDataSourceWidget::needsRecreate()
{
    bool show_counts = ds_man_.loadWidgetShowCounts();

    if (last_show_counts_ != show_counts)
        return true;

    string ds_content_name;

    unsigned int ds_to_show=0;
    for (auto& cnt_it : src_.numInsertedSummedLinesMap())
    {
        ds_content_name = cnt_it.first;

        if (!content_labels_.count(ds_content_name)) // not yet created
            return true;

        ++ds_to_show;
    }

    return ds_to_show != content_labels_.size(); // check that not too many
}

void DBDataSourceWidget::recreateWidgets()
{
    bool show_counts = ds_man_.loadWidgetShowCounts();

    loginf << "DBDataSourceWidget " << src_.name() << ": recreateWidgets: show_counts " << show_counts;

    QLayoutItem* child;
    while ((child = grid_layout_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    load_check_ = nullptr;
    content_labels_.clear();
    loaded_cnt_labels_.clear();
    total_cnt_labels_.clear();

    // update load check
    load_check_ = new QCheckBox(src_.name().c_str());
    connect(load_check_, &QCheckBox::clicked, this, &DBDataSourceWidget::loadingChangedSlot);

    unsigned int row = 0;

    grid_layout_->addWidget(load_check_, row, 0, 1, 4);
    ++row;

    if (show_counts)
    {
        string ds_content_name;

        for (auto& cnt_it : src_.numInsertedSummedLinesMap())
        {
            ds_content_name = cnt_it.first;

            assert (!content_labels_.count(ds_content_name));
            assert (!loaded_cnt_labels_.count(ds_content_name));
            assert (!total_cnt_labels_.count(ds_content_name));

            content_labels_[ds_content_name] = new QLabel(ds_content_name.c_str());
            content_labels_.at(ds_content_name)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            loaded_cnt_labels_[ds_content_name] = new QLabel(QString::number(src_.numLoaded(ds_content_name)));
            loaded_cnt_labels_.at(ds_content_name)->setAlignment(Qt::AlignRight);
            loaded_cnt_labels_.at(ds_content_name)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            total_cnt_labels_[ds_content_name] = new QLabel(QString::number(cnt_it.second));
            total_cnt_labels_.at(ds_content_name)->setAlignment(Qt::AlignRight);
            total_cnt_labels_.at(ds_content_name)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            grid_layout_->addWidget(content_labels_.at(ds_content_name), row, 1);
            grid_layout_->addWidget(loaded_cnt_labels_.at(ds_content_name), row, 2);
            grid_layout_->addWidget(total_cnt_labels_.at(ds_content_name), row, 3);
        }
    }

    for(int c=0; c < grid_layout_->columnCount(); c++)
    {
        if (c == 0) // first is half placeholder
            grid_layout_->setColumnStretch(c, 1);
        else
            grid_layout_->setColumnStretch(c, 2);
    }

    last_show_counts_ = show_counts;

    update();
}

void DBDataSourceWidget::updateWidgets()
{
    loginf << "DBDataSourceWidget: updateWidgets";

    bool show_counts = ds_man_.loadWidgetShowCounts();

    assert (load_check_);
    load_check_->setText(src_.name().c_str());

    load_check_->setChecked(src_.loadingWanted());

    // ds_id -> line str ->(ip, port)
    std::map<unsigned int, std::map<std::string, std::pair<std::string, unsigned int>>> net_lines =
            ds_man_.getNetworkLines();

    string tooltip;

    //    if (net_lines.count(ds_id))
    //    {
    //        QHBoxLayout* button_lay = new QHBoxLayout();

    //        unsigned int last_line_number=0, current_line_number=0;

    //        for (auto& line_it : net_lines.at(ds_id))
    //        {
    //            current_line_number = String::getAppendedInt(line_it.first);
    //            assert (current_line_number >= 1 && current_line_number <= 4);

    //            if (current_line_number > 1 && (current_line_number - last_line_number) > 1)
    //            {
    //                // space to be inserted

    //                unsigned int num_spaces = current_line_number - last_line_number - 1;

    //                for (unsigned int cnt=0; cnt < num_spaces; ++cnt)
    //                    button_lay->addSpacing(button_size+2);
    //            }

    //            QPushButton* button = new QPushButton (line_it.first.c_str());
    //            button->setFixedSize(button_size,button_size);
    //            button->setCheckable(true);
    //            button->setDown(current_line_number == 1);

    //            QPalette pal = button->palette();

    //            if (current_line_number == 1)
    //                pal.setColor(QPalette::Button, QColor(Qt::green));
    //            else
    //                pal.setColor(QPalette::Button, QColor(Qt::yellow));

    //            button->setAutoFillBackground(true);
    //            button->setPalette(pal);
    //            button->update();
    //            //button->setDisabled(line_cnt != 0);

    //            if (current_line_number == 1)
    //                tooltip = "Connected";
    //            else
    //                tooltip = "Not Connected";

    //            tooltip += "\nIP: "+line_it.second.first+":"
    //                                     +to_string(line_it.second.second);

    //            button->setToolTip(tooltip.c_str());

    //            button_lay->addWidget(button);

    //            last_line_number = current_line_number;
    //        }

    //        type_layout_->addLayout(button_lay, row, col_start+3, // 2 for start
    //                                Qt::AlignTop | Qt::AlignLeft);
    //    }

    //            ++row;


    if (show_counts)
    {
        string ds_content_name;
        for (auto& cnt_it : src_.numInsertedSummedLinesMap())
        {
            ds_content_name = cnt_it.first;

            // content label

            assert (content_labels_.count(ds_content_name));
            assert (loaded_cnt_labels_.count(ds_content_name));
            assert (total_cnt_labels_.count(ds_content_name));

            loaded_cnt_labels_[ds_content_name]->setText(QString::number(src_.numLoaded(ds_content_name)));
            total_cnt_labels_[ds_content_name]->setText(QString::number(cnt_it.second));
        }
    }
}

void DBDataSourceWidget::loadingChangedSlot()
{

}

}
