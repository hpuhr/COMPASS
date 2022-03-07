/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "datasourcesloadwidget.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "datasourcemanager.h"
#include "global.h"
#include "stringconv.h"
#include "viewmanager.h"
#include "number.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace std;
using namespace Utils;
using namespace Utils::String;

DataSourcesLoadWidget::DataSourcesLoadWidget(DataSourceManager& ds_man)
    : ds_man_(ds_man)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    // data sources, per type

    type_layout_ = new QGridLayout();

    main_layout->addLayout(type_layout_);

    main_layout->addStretch();

    // associations

    QHBoxLayout* assoc_layout = new QHBoxLayout();

    QLabel* assoc_label = new QLabel("Associations");
    assoc_label->setFont(font_bold);
    assoc_layout->addWidget(assoc_label);


    associations_label_ = new QLabel("None");
    //associations_label_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    assoc_layout->addWidget(associations_label_);

    assoc_layout->addStretch();

    main_layout->addLayout(assoc_layout);

    //    update();

    //    main_layout->addStretch();

    //    // limit stuff
    //    bool use_limit = dbo_manager_.useLimit();
    //    limit_check_ = new QCheckBox("Use Limit");
    //    limit_check_->setChecked(use_limit);
    //    connect(limit_check_, &QCheckBox::clicked, this, &DataSourcesLoadWidget::toggleUseLimit);
    //    main_layout->addWidget(limit_check_);


    //    QHBoxLayout* bottom_layout = new QHBoxLayout();


    //    bottom_layout->addStretch();

    //    main_layout->addLayout(bottom_layout);

    update();

    setLayout(main_layout);
}

DataSourcesLoadWidget::~DataSourcesLoadWidget() {}


void DataSourcesLoadWidget::loadDSTypeChangedSlot()
{
    QCheckBox* box = dynamic_cast<QCheckBox*>(QObject::sender());
    assert (box);

    string ds_type_name =box->property("DSType").toString().toStdString();

    bool load = box->checkState() == Qt::Checked;

    loginf << "DataSourcesLoadWidget: loadDSTypeChangedSlot: ds_type " << ds_type_name << " load " << load;

    COMPASS::instance().dataSourceManager().dsTypeLoadingWanted(ds_type_name, load);
}

void DataSourcesLoadWidget::loadDSChangedSlot()
{
    QCheckBox* box = dynamic_cast<QCheckBox*>(QObject::sender());
    assert (box);

    unsigned int ds_id = box->property("DS ID").toUInt();

    bool load = box->checkState() == Qt::Checked;

    loginf << "DataSourcesLoadWidget: loadDSChangedSlot: ds_id " << ds_id << " load " << load;

    ds_man_.dbDataSource(ds_id).loadingWanted(load);
}


//void DataSourcesLoadWidget::toggleUseLimit()
//{
//    assert(limit_check_);
//    assert(limit_widget_);
//    assert(limit_min_edit_);
//    assert(limit_max_edit_);

//    bool checked = limit_check_->checkState() == Qt::Checked;
//    logdbg << "DataSourcesLoadWidget: toggleUseLimit: setting use limit to " << checked;
//    dbo_manager_.useLimit(checked);

//    if (checked)
//        limit_widget_->show();
//    else
//        limit_widget_->hide();

//    limit_min_edit_->setEnabled(checked);
//    limit_max_edit_->setEnabled(checked);
//}

//void DataSourcesLoadWidget::limitMinChanged()
//{
//    assert(limit_min_edit_);

//    if (limit_min_edit_->text().size() == 0)
//        return;

//    unsigned int min = std::stoul(limit_min_edit_->text().toStdString());
//    dbo_manager_.limitMin(min);
//}
//void DataSourcesLoadWidget::limitMaxChanged()
//{
//    assert(limit_max_edit_);

//    if (limit_max_edit_->text().size() == 0)
//        return;

//    unsigned int max = std::stoul(limit_max_edit_->text().toStdString());
//    dbo_manager_.limitMax(max);
//}

void DataSourcesLoadWidget::update()
{
    logdbg << "DataSourcesLoadWidget: update: num data sources " << ds_man_.dbDataSources().size();

    bool clear_required = false;

    for (const auto& ds_it : ds_man_.dbDataSources())
    {
        if (!ds_boxes_.count(ds_it->name()))
        {
            loginf << "DataSourcesLoadWidget: update: ds_box " << ds_it->name() << " missing ";

            clear_required = true;
            break;
        }
        // check content widget exist

        if (!show_counts_ || !ds_it->hasNumInserted()) // no counts or no data
            continue;

        for (auto& cnt_it : ds_it->numInsertedSummedLinesMap())
        {
            if (!ds_content_boxes_.count(ds_it->name()) || !ds_content_boxes_.at(ds_it->name()).count(cnt_it.first))
            {
                loginf << "DataSourcesLoadWidget: update: ds_content_boxes " << cnt_it.first << " missing ";

                clear_required = true;
                break;
            }
        }

    }

    if (clear_required)
        clearAndCreateContent();
    else
        updateExistingContent();

// TODO move this
    DBContentManager& dbo_man = COMPASS::instance().dbContentManager();

    assert(associations_label_);
    if (dbo_man.hasAssociations())
    {
        std::string tmp = "From " + dbo_man.associationsID();
        associations_label_->setText(tmp.c_str());
    }
    else
        associations_label_->setText("None");
}

void DataSourcesLoadWidget::clearAndCreateContent()
{
    loginf << "DataSourcesLoadWidget: clearAndCreateContent";

    // remove all previous
    while (QLayoutItem* item = type_layout_->takeAt(0))
    {
        if (item->layout())
        {
            while (QLayoutItem* item2 = item->layout()->takeAt(0))
            {
                if (item2->widget())
                    delete item2->widget();

                delete item2;
            }
        }

        if (item->widget())
            delete item->widget();

        delete item;
    }

    ds_type_boxes_.clear();
    ds_boxes_.clear();
    ds_content_boxes_.clear();
    ds_content_loaded_labels_.clear();
    ds_content_total_labels_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    unsigned int row = 0;
    unsigned int col_start = 0;
    unsigned int num_col_per_dstype = 5; // add 1 for spacing
    unsigned int dstype_col = 0;
    unsigned int dstyp_cnt = 0;

    unsigned int ds_id;
    string ds_name;
    string ds_content_name;

    unsigned int button_size = 28;

    // ds_id -> line str ->(ip, port)
    std::map<unsigned int, std::map<std::string, std::pair<std::string, unsigned int>>> net_lines =
            ds_man_.getNetworkLines();

    string tooltip;

    //DBContentManager& dbo_man = COMPASS::instance().dbContentManager();
    bool ds_found;

    for (auto& ds_type_name : DataSourceManager::data_source_types_)
    {
        logdbg << "DataSourcesLoadWidget: clearAndCreateContent: typ " << ds_type_name << " cnt " << dstyp_cnt;

        if (ds_type_name == "MLAT" || ds_type_name == "Tracker" || ds_type_name == "Other")  // break into next column
        {
            row = 0;
            dstype_col++;
            col_start = dstype_col * num_col_per_dstype;
        }

        QCheckBox* dstyp_box = new QCheckBox(ds_type_name.c_str());
        dstyp_box->setFont(font_bold);
        dstyp_box->setChecked(ds_man_.dsTypeLoadingWanted(ds_type_name));
        dstyp_box->setProperty("DSType", ds_type_name.c_str());

        connect(dstyp_box, &QCheckBox::clicked, this,
                &DataSourcesLoadWidget::loadDSTypeChangedSlot);

        type_layout_->addWidget(dstyp_box, row, col_start, 1, num_col_per_dstype, Qt::AlignTop | Qt::AlignLeft);

        assert (!ds_type_boxes_.count(ds_type_name));
        ds_type_boxes_[ds_type_name] = dstyp_box;

        ++row;

        ds_found = false;

        for (const auto& ds_it : ds_man_.dbDataSources())
        {
            if (ds_it->dsType() != ds_type_name)
                continue;

            ds_found = true;

            ds_id = Number::dsIdFrom(ds_it->sac(), ds_it->sic());
            ds_name = ds_it->name();
            loginf << "DataSourcesLoadWidget: clearAndCreateContent: create '"
                   << ds_it->dsType() << "' '" << ds_name << "'";

            QCheckBox* ds_box = new QCheckBox(ds_name.c_str());
            ds_box->setChecked(ds_it->loadingWanted());
            ds_box->setProperty("DS ID", ds_id);

            connect(ds_box, &QCheckBox::clicked, this,
                    &DataSourcesLoadWidget::loadDSChangedSlot);

            type_layout_->addWidget(ds_box, row, col_start, 1, 2, //num_col_per_dstype-1,
                                    Qt::AlignTop | Qt::AlignLeft);

            assert (!ds_boxes_.count(ds_name));
            ds_boxes_[ds_name] = ds_box;

            // Line Buttons
            if (net_lines.count(ds_id))
            {
                QHBoxLayout* button_lay = new QHBoxLayout();

                unsigned int last_line_number=0, current_line_number=0;

                for (auto& line_it : net_lines.at(ds_id))
                {
                    current_line_number = String::getAppendedInt(line_it.first);
                    assert (current_line_number >= 1 && current_line_number <= 4);

                    if (current_line_number > 1 && (current_line_number - last_line_number) > 1)
                    {
                        // space to be inserted

                        unsigned int num_spaces = current_line_number - last_line_number - 1;

                        for (unsigned int cnt=0; cnt < num_spaces; ++cnt)
                            button_lay->addSpacing(button_size);
                    }

                    QPushButton* button = new QPushButton (line_it.first.c_str());
                    button->setFixedSize(button_size,button_size);
                    button->setCheckable(true);
                    button->setDown(current_line_number == 1);

                    QPalette pal = button->palette();

                    if (current_line_number == 1)
                        pal.setColor(QPalette::Button, QColor(Qt::green));
                    else
                        pal.setColor(QPalette::Button, QColor(Qt::yellow));

                    button->setAutoFillBackground(true);
                    button->setPalette(pal);
                    button->update();
                    //button->setDisabled(line_cnt != 0);

                    if (current_line_number == 1)
                        tooltip = "Connected";
                    else
                        tooltip = "Not Connected";

                    tooltip += "\nIP: "+line_it.second.first+":"
                                 +to_string(line_it.second.second);

                    button->setToolTip(tooltip.c_str());

                    button_lay->addWidget(button);

                    last_line_number = current_line_number;
                }

                type_layout_->addLayout(button_lay, row, col_start+3, // 2 for start
                                        Qt::AlignTop | Qt::AlignLeft);
            }

            ++row;

            if (show_counts_)
            {
                for (auto& cnt_it : ds_it->numInsertedSummedLinesMap())
                {
                    ds_content_name = cnt_it.first;

                    // content checkbox

                    QLabel* dbcont_box = new QLabel(ds_content_name.c_str());

                    type_layout_->addWidget(dbcont_box, row, col_start+1,
                                            Qt::AlignTop | Qt::AlignRight);

                    assert (!ds_content_boxes_.count(ds_name) || !ds_content_boxes_.at(ds_name).count(ds_content_name));
                    ds_content_boxes_[ds_name][ds_content_name] = dbcont_box;

                    // ds content loaded label

                    QLabel* ds_content_loaded_label = new QLabel(QString::number(ds_it->numLoaded(ds_content_name)));

                    type_layout_->addWidget(ds_content_loaded_label, row, col_start+2,
                                            Qt::AlignTop | Qt::AlignRight);

                    assert (!ds_content_loaded_labels_.count(ds_name)
                            || !ds_content_loaded_labels_.at(ds_name).count(ds_content_name));
                    ds_content_loaded_labels_[ds_name][ds_content_name] = ds_content_loaded_label;

                    // ds content num inserted label

                    QLabel* ds_content_total_label = new QLabel(QString::number(cnt_it.second));

                    type_layout_->addWidget(ds_content_total_label, row, col_start+3,
                                            Qt::AlignTop | Qt::AlignRight);

                    assert (!ds_content_total_labels_.count(ds_name)
                            || !ds_content_total_labels_.at(ds_name).count(ds_content_name));
                    ds_content_total_labels_[ds_name][ds_content_name] = ds_content_total_label;
                    ++row;
                }
            }
        }

        if (!ds_found)
        {
            dstyp_box->setChecked(false);
            dstyp_box->setDisabled(true);
        }

        dstyp_cnt++;
    }
}

void DataSourcesLoadWidget::updateExistingContent()
{
    logdbg << "DataSourcesLoadWidget: updateExistingContent";

    for (auto& ds_typ_it : ds_type_boxes_)
    {
        logdbg << "DataSourcesLoadWidget: updateExistingContent: ds_typ " << ds_typ_it.first
               << " " << ds_man_.dsTypeLoadingWanted(ds_typ_it.first);
        ds_typ_it.second->setChecked(ds_man_.dsTypeLoadingWanted(ds_typ_it.first));
    }

    string ds_name;
    string ds_content_name;

    for (const auto& ds_it : ds_man_.dbDataSources())
    {
        //loginf << row << " '" << ds_it->dsType() << "' '" << dstype << "'";

        ds_name = ds_it->name();

        assert (ds_boxes_.count(ds_name));
        ds_boxes_.at(ds_name)->setChecked(ds_it->loadingWanted());
        // ds_boxes_[ds_name] // checkbox

        logdbg << "DataSourcesLoadWidget: updateExistingContent: ds " << ds_name
               << " " << ds_it->loadingWanted();

        if (show_counts_)
        {
            for (auto& cnt_it : ds_it->numInsertedSummedLinesMap())
            {
                ds_content_name = cnt_it.first;

                // content checkbox

                assert (ds_content_boxes_.count(ds_name) && ds_content_boxes_.at(ds_name).count(ds_content_name));
                // ds_content_boxes_[ds_name][ds_content_name] content checkbox

                // ds content loaded label
                assert (ds_content_loaded_labels_.count(ds_name)
                        && ds_content_loaded_labels_.at(ds_name).count(ds_content_name));
                ds_content_loaded_labels_[ds_name][ds_content_name]->setText(
                            QString::number(ds_it->numLoaded(ds_content_name)));

                // ds content total label

                assert (ds_content_total_labels_.count(ds_name)
                        && ds_content_total_labels_.at(ds_name).count(ds_content_name));
                ds_content_total_labels_[ds_name][ds_content_name]->setText(QString::number(cnt_it.second));
            }
        }
    }
}

