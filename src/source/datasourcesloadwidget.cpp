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
#include "source/dbdatasourcewidget.h"
#include "datasourcemanager.h"
#include "global.h"
#include "stringconv.h"
#include "number.h"
#include "files.h"
#include "timeconv.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>

using namespace std;
using namespace Utils;

DataSourcesLoadWidget::DataSourcesLoadWidget(DataSourceManager& ds_man)
    : ds_man_(ds_man)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* sub_layout = new QVBoxLayout();
    sub_layout->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);
    scroll_area->setContentsMargins(0, 0, 0, 0);
    //    scroll_area->setStyleSheet(" QAbstractScrollArea { background-color: transparent; }"
    //                               " QWidget#scrollAreaWidgetContents{ background-color: transparent; }");

    sub_layout->addWidget(scroll_area);

    QWidget* sub_widget = new QWidget();
    sub_widget->setContentsMargins(0, 0, 0, 0);
    //sub_widget->setStyleSheet("background-color: white"); // works but overrides line colors

    scroll_area->setWidget(sub_widget);

    QVBoxLayout* main_layout = new QVBoxLayout();
    //main_layout->setContentsMargins(1, 1, 1, 1);

            // button

    QHBoxLayout* button_layout = new QHBoxLayout();
    //button_layout->setContentsMargins(0, 0, 0, 0);

    button_layout->addStretch();

    QPushButton* edit_button = new QPushButton();
    edit_button->setIcon(QIcon(Files::getIconFilepath("edit.png").c_str()));
    edit_button->setFixedSize(UI_ICON_SIZE);
    edit_button->setFlat(UI_ICON_BUTTON_FLAT);
    edit_button->setToolTip(tr("Data Source Options"));
    connect (edit_button, &QPushButton::clicked, this, &DataSourcesLoadWidget::editClickedSlot);
    button_layout->addWidget(edit_button);

    main_layout->addLayout(button_layout);

    //    main_layout->addLayout(vlay);

    //    main_layout->addStretch();

    //    QHBoxLayout* vlay = new QHBoxLayout();
    //    vlay->setContentsMargins(0, 0, 0, 0);

            // data sources, per type

    type_layout_ = new QGridLayout();

    main_layout->addLayout(type_layout_);

    main_layout->addStretch();

            // associations

    QHBoxLayout* assoc_layout = new QHBoxLayout();

            // time
    QLabel* ts_label = new QLabel("Timestamps");
    ts_label->setFont(font_bold);
    assoc_layout->addWidget(ts_label);

    assoc_layout->addWidget(new QLabel("Min"));

    ts_min_label_ = new QLabel("None");
    assoc_layout->addWidget(ts_min_label_);

    assoc_layout->addWidget(new QLabel("Max"));

    ts_max_label_ = new QLabel("None");
    assoc_layout->addWidget(ts_max_label_);

    assoc_layout->addStretch();

            // assoc
    QLabel* assoc_label = new QLabel("Associations");
    assoc_label->setFont(font_bold);
    assoc_layout->addWidget(assoc_label);

    associations_label_ = new QLabel("None");
    //associations_label_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    assoc_layout->addWidget(associations_label_);

    assoc_layout->addStretch();

    main_layout->addLayout(assoc_layout);

    updateContent();

    sub_widget->setLayout(main_layout);

    setLayout(sub_layout);

            // menu
    QAction* sel_dstyp_action = edit_menu_.addAction("Select All DSTypes");
    connect(sel_dstyp_action, &QAction::triggered, this, &DataSourcesLoadWidget::selectAllDSTypesSlot);

    QAction* desel_dstyp_action = edit_menu_.addAction("Deselect All DSTypes");
    connect(desel_dstyp_action, &QAction::triggered, this, &DataSourcesLoadWidget::deselectAllDSTypesSlot);

    edit_menu_.addSeparator();

    QMenu* select_ds = edit_menu_.addMenu("Select Data Sources");

    QAction* sel_ds_action = select_ds->addAction("All");
    connect(sel_ds_action, &QAction::triggered, this, &DataSourcesLoadWidget::selectAllDataSourcesSlot);

    for (const auto& ds_type_it : ds_man_.data_source_types_)
    {
        QAction* action = select_ds->addAction(("From "+ds_type_it).c_str());
        action->setProperty("ds_type", ds_type_it.c_str());
        connect(action, &QAction::triggered, this, &DataSourcesLoadWidget::selectDSTypeSpecificDataSourcesSlot);
    }

    QMenu* deselect_ds = edit_menu_.addMenu("Deselect Data Sources");

    QAction* desel_ds_action = deselect_ds->addAction("All");
    connect(desel_ds_action, &QAction::triggered, this, &DataSourcesLoadWidget::deselectAllDataSourcesSlot);


    for (const auto& ds_type_it : ds_man_.data_source_types_)
    {
        QAction* action = deselect_ds->addAction(("From "+ds_type_it).c_str());
        action->setProperty("ds_type", ds_type_it.c_str());
        connect(action, &QAction::triggered, this, &DataSourcesLoadWidget::deselectDSTypeSpecificDataSourcesSlot);
    }

    edit_menu_.addSeparator();

    QMenu* set_lines = edit_menu_.addMenu("Set Line");

    QAction* desel_line_action = set_lines->addAction("Deselect All");
    connect(desel_line_action, &QAction::triggered, this, &DataSourcesLoadWidget::deselectAllLinesSlot);

    for (unsigned int cnt=0; cnt < 4; ++cnt)
    {
        QAction* desel_line_action = set_lines->addAction(("Select "+String::lineStrFrom(cnt)).c_str());
        desel_line_action->setProperty("line_id", cnt);
        connect(desel_line_action, &QAction::triggered, this, &DataSourcesLoadWidget::selectSpecificLineSlot);
    }

    edit_menu_.addSeparator();

    QAction* show_cnt_action = edit_menu_.addAction("Toggle Show Counts");
    connect(show_cnt_action, &QAction::triggered, this, &DataSourcesLoadWidget::toogleShowCountsSlot);
}

DataSourcesLoadWidget::~DataSourcesLoadWidget()
{
}


void DataSourcesLoadWidget::loadDSTypeChangedSlot()
{
    QCheckBox* box = dynamic_cast<QCheckBox*>(QObject::sender());
    assert (box);

    string ds_type_name =box->property("DSType").toString().toStdString();

    bool load = box->checkState() == Qt::Checked;

    loginf << "DataSourcesLoadWidget: loadDSTypeChangedSlot: ds_type " << ds_type_name << " load " << load;

    COMPASS::instance().dataSourceManager().dsTypeLoadingWanted(ds_type_name, load);
}

//void DataSourcesLoadWidget::loadDSChangedSlot()
//{
//    QCheckBox* box = dynamic_cast<QCheckBox*>(QObject::sender());
//    assert (box);

//    unsigned int ds_id = box->property("DS ID").toUInt();

//    bool load = box->checkState() == Qt::Checked;

//    loginf << "DataSourcesLoadWidget: loadDSChangedSlot: ds_id " << ds_id << " load " << load;

//    ds_man_.dbDataSource(ds_id).loadingWanted(load);
//}

void DataSourcesLoadWidget::editClickedSlot()
{
    loginf << "DataSourcesLoadWidget: editClickedSlot";

    edit_menu_.exec(QCursor::pos());
}

void DataSourcesLoadWidget::selectAllDSTypesSlot()
{
    loginf << "DataSourcesLoadWidget: selectAllDSTypesSlot";

    ds_man_.selectAllDSTypes();

}
void DataSourcesLoadWidget::deselectAllDSTypesSlot()
{
    loginf << "DataSourcesLoadWidget: deselectAllDSTypesSlot";

    ds_man_.deselectAllDSTypes();
}

void DataSourcesLoadWidget::selectAllDataSourcesSlot()
{
    loginf << "DataSourcesLoadWidget: selectAllDataSourcesSlot";

    ds_man_.selectAllDataSources();
}
void DataSourcesLoadWidget::deselectAllDataSourcesSlot()
{
    loginf << "DataSourcesLoadWidget: deselectAllDataSourcesSlot";

    ds_man_.deselectAllDataSources();
}

void DataSourcesLoadWidget::selectDSTypeSpecificDataSourcesSlot()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    string ds_type = action->property("ds_type").toString().toStdString();

    loginf << "DataSourcesLoadWidget: selectDSTypeSpecificDataSourcesSlot: ds_type '" << ds_type << "'";

    ds_man_.selectDSTypeSpecificDataSources(ds_type);
}

void DataSourcesLoadWidget::deselectDSTypeSpecificDataSourcesSlot()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    string ds_type = action->property("ds_type").toString().toStdString();

    loginf << "DataSourcesLoadWidget: deselectDSTypeSpecificDataSourcesSlot: ds_type '" << ds_type << "'";

    ds_man_.deselectDSTypeSpecificDataSources(ds_type);
}

void DataSourcesLoadWidget::deselectAllLinesSlot()
{
    loginf << "DataSourcesLoadWidget: deselectAllLinesSlot";

    ds_man_.deselectAllLines();
}

void DataSourcesLoadWidget::selectSpecificLineSlot()
{
    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    unsigned int line_id = action->property("line_id").toUInt();

    loginf << "DataSourcesLoadWidget: selectSpecificLineSlot: line_id " << line_id;

    ds_man_.selectSpecificLineSlot(line_id);
}

void DataSourcesLoadWidget::toogleShowCountsSlot()
{
    loginf << "DataSourcesLoadWidget: toogleShowCountsSlot";

    ds_man_.config().load_widget_show_counts_ = !ds_man_.config().load_widget_show_counts_;

    updateContent();
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

void DataSourcesLoadWidget::updateContent(bool recreate_required)
{
    logdbg << "DataSourcesLoadWidget: updateContent: recreate_required " << recreate_required
           << " num data sources " << ds_man_.dbDataSources().size();

    for (auto& ds_type_it : ds_type_boxes_)
        ds_type_it.second->setChecked(ds_man_.dsTypeLoadingWanted(ds_type_it.first));

    if (ds_widgets_.size() != ds_man_.dbDataSources().size()) // check if same size
        recreate_required = true;
    else // check each one
    {
        for (const auto& ds_it : ds_man_.dbDataSources())
        {
            if (!ds_widgets_.count(ds_it->name()))
            {
                logdbg << "DataSourcesLoadWidget: updateContent: ds_box " << ds_it->name() << " missing ";

                recreate_required = true;
                break;
            }
        }
    }

    logdbg << "DataSourcesLoadWidget: updateContent: recreate_required " << recreate_required;

    if (recreate_required)
    {
        clearAndCreateContent();
    }
    else
    {
        for (auto& ds_widget_it : ds_widgets_)
            ds_widget_it.second->updateContent();
    }

            // TODO move this
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    assert (ts_min_label_);
    assert (ts_max_label_);

    if (dbcont_man.hasMinMaxTimestamp())
    {
        ts_min_label_->setText(Time::toString(get<0>(dbcont_man.minMaxTimestamp()), 0).c_str());
        ts_max_label_->setText(Time::toString(get<1>(dbcont_man.minMaxTimestamp()), 0).c_str());
    }
    else
    {
        ts_min_label_->setText("None");
        ts_max_label_->setText("None");
    }

    assert(associations_label_);
    if (dbcont_man.hasAssociations())
    {
        std::string tmp = "From " + dbcont_man.associationsID();
        associations_label_->setText(tmp.c_str());
    }
    else
        associations_label_->setText("None");

    arrangeSourceWidgetWidths();
}

void DataSourcesLoadWidget::clear()
{
    // delete dstype boxes
    for (auto& ds_type_box_it : ds_type_boxes_)
        delete ds_type_box_it.second;
    ds_type_boxes_.clear();

    ds_widgets_.clear(); // delete by data sources

            // delete all previous
    QLayoutItem* child;
    while (!type_layout_->isEmpty() && (child = type_layout_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }
}

void DataSourcesLoadWidget::arrangeSourceWidgetWidths()
{
    logdbg << "DataSourcesLoadWidget: arrangeSourceWidgetWidths";

    unsigned int min_width = 0;

    for (auto& widget_it : ds_widgets_)
        min_width = max(min_width, widget_it.second->getLabelMinWidth());

    if (min_width)
    {
        logdbg << "DataSourcesLoadWidget: arrangeSourceWidgetWidths: setting width " << min_width;

        for (auto& widget_it : ds_widgets_)
            widget_it.second->updateLabelMinWidth(min_width);
    }
}

void DataSourcesLoadWidget::clearAndCreateContent()
{
    logdbg << "DataSourcesLoadWidget: clearAndCreateContent";

    clear();

    QFont font_bold;
    font_bold.setBold(true);
    font_bold.setPointSize(ds_man_.config().ds_font_size_);

    unsigned int row = 0;
    unsigned int col = 0;
    //unsigned int dstype_col = 0;
    unsigned int dstyp_cnt = 0;

    unsigned int ds_id;
    string ds_name;
    string ds_content_name;

    using namespace dbContent;

            //DBContentManager& dbo_man = COMPASS::instance().dbContentManager();
    bool ds_found;

    logdbg << "DataSourcesLoadWidget: clearAndCreateContent: iterating data source types";

    for (auto& ds_type_name : DataSourceManager::data_source_types_)
    {
        logdbg << "DataSourcesLoadWidget: clearAndCreateContent: typ " << ds_type_name << " cnt " << dstyp_cnt;

        if (ds_type_name == "MLAT" || ds_type_name == "Tracker")  // break into next column
        {
            row = 0;
            col++;
        }

        QCheckBox* dstyp_box = new QCheckBox(ds_type_name.c_str());
        dstyp_box->setFont(font_bold);
        dstyp_box->setChecked(ds_man_.dsTypeLoadingWanted(ds_type_name));
        dstyp_box->setProperty("DSType", ds_type_name.c_str());

        connect(dstyp_box, &QCheckBox::clicked,
                this, &DataSourcesLoadWidget::loadDSTypeChangedSlot);

        dstyp_box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

        type_layout_->addWidget(dstyp_box, row, col);

        assert (!ds_type_boxes_.count(ds_type_name));
        ds_type_boxes_[ds_type_name] = dstyp_box;

        ++row;

        ds_found = false;

        logdbg << "DataSourcesLoadWidget: clearAndCreateContent: data sources for type " << ds_type_name;

        for (const auto& ds_it : ds_man_.dbDataSources())
        {
            if (ds_it->dsType() != ds_type_name)
                continue;

            ds_found = true;

            ds_id = Number::dsIdFrom(ds_it->sac(), ds_it->sic());
            ds_name = ds_it->name();
            logdbg << "DataSourcesLoadWidget: clearAndCreateContent: create '" << ds_it->dsType()
                   << "' '" << ds_name << "'";

            std::function<bool()> get_use_ds_func =
                [&ds_it] () { return ds_it->loadingWanted(); };
            std::function<void(bool)> set_use_ds_func  =
                [&ds_it] (bool value) { return ds_it->loadingWanted(value); };
            std::function<bool(unsigned int)> get_use_ds_line_func =
                [&ds_it] (unsigned int line_id) { return ds_it->lineLoadingWanted(line_id); };
            std::function<void(unsigned int, bool)> set_use_ds_line_func  =
                [&ds_it] (unsigned int line_id, bool value) { return ds_it->lineLoadingWanted(line_id, value); };

            std::function<bool()> show_counts_func =
                [this] () { return ds_man_.config().load_widget_show_counts_; };


            DBDataSourceWidget* ds_widget = new DBDataSourceWidget(
                *ds_it, get_use_ds_func, set_use_ds_func,
                get_use_ds_line_func, set_use_ds_line_func,
                show_counts_func);

            type_layout_->addWidget(ds_widget, row, col);

            assert (!ds_widgets_.count(ds_name));
            ds_widgets_[ds_name] = ds_widget;

            ++row;
        }

        if (!ds_found)
        {
            dstyp_box->setChecked(false);
            dstyp_box->setDisabled(true);
        }

        dstyp_cnt++;
    }

    logdbg << "DataSourcesLoadWidget: clearAndCreateContent: setting columns";

    for(int c=0; c < type_layout_->columnCount(); c++)
        type_layout_->setColumnStretch(c,1);

    logdbg << "DataSourcesLoadWidget: clearAndCreateContent: done";
}

//void DataSourcesLoadWidget::updateExistingContent()
//{
//    logdbg << "DataSourcesLoadWidget: updateExistingContent";

//    for (auto& ds_typ_it : ds_type_boxes_)
//    {
//        logdbg << "DataSourcesLoadWidget: updateExistingContent: ds_typ " << ds_typ_it.first
//               << " " << ds_man_.dsTypeLoadingWanted(ds_typ_it.first);
//        ds_typ_it.second->setChecked(ds_man_.dsTypeLoadingWanted(ds_typ_it.first));
//    }

//    string ds_name;
//    string ds_content_name;

//    bool show_counts = ds_man_.loadWidgetShowCounts();

//    for (const auto& ds_it : ds_man_.dbDataSources())
//    {
//        //loginf << row << " '" << ds_it->dsType() << "' '" << dstype << "'";

//        ds_name = ds_it->name();

//        assert (ds_boxes_.count(ds_name));
//        ds_boxes_.at(ds_name)->setChecked(ds_it->loadingWanted());
//        // ds_boxes_[ds_name] // checkbox

//        logdbg << "DataSourcesLoadWidget: updateExistingContent: ds " << ds_name
//               << " " << ds_it->loadingWanted();

//        if (show_counts)
//        {
//            for (auto& cnt_it : ds_it->numInsertedSummedLinesMap())
//            {
//                ds_content_name = cnt_it.first;

//                // content checkbox

//                assert (ds_content_boxes_.count(ds_name) && ds_content_boxes_.at(ds_name).count(ds_content_name));
//                // ds_content_boxes_[ds_name][ds_content_name] content checkbox

//                // ds content loaded label
//                assert (ds_content_loaded_labels_.count(ds_name)
//                        && ds_content_loaded_labels_.at(ds_name).count(ds_content_name));
//                ds_content_loaded_labels_[ds_name][ds_content_name]->setText(
//                            QString::number(ds_it->numLoaded(ds_content_name)));

//                // ds content total label

//                assert (ds_content_total_labels_.count(ds_name)
//                        && ds_content_total_labels_.at(ds_name).count(ds_content_name));
//                ds_content_total_labels_[ds_name][ds_content_name]->setText(QString::number(cnt_it.second));
//            }
//        }
//    }
//}

