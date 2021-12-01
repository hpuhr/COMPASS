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

#include "dbobjectmanagerloadwidget.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "compass.h"
#include "dbobject.h"
#include "dbobjectinfowidget.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "global.h"
#include "stringconv.h"
#include "viewmanager.h"

using namespace std;
using namespace Utils::String;

DBObjectManagerLoadWidget::DBObjectManagerLoadWidget(DBObjectManager& object_manager)
    : dbo_manager_(object_manager)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

//    QLabel* main_label = new QLabel("Database Objects");
//    main_label->setFont(font_bold);
//    main_layout->addWidget(main_label);

    // data sources, per type

    type_layout_ = new QGridLayout();

//    QGridLayout* dstypes_lay = new QGridLayout();

//    unsigned int row = 0, col = 0;
//    for (auto& dstyp_it : DBObjectManager::data_source_types_)
//    {
//        QVBoxLayout* lay = new QVBoxLayout();

//        QLabel* dstyp_label = new QLabel(dstyp_it.c_str());
//        dstyp_label->setFont(font_bold);

//        lay->addWidget(dstyp_label);

//        QGridLayout* dstyp_lay = new QGridLayout();
//        type_layouts_[dstyp_it] = dstyp_lay;
//        lay->addLayout(dstyp_lay);

//        // void addLayout(QLayout *, int row, int column, int rowSpan, int columnSpan, Qt::Alignment = Qt::Alignment());
//        if (dstyp_it == "Radar") // span 2 rows
//        {
//            dstypes_lay->addLayout(lay, row, col, 2, 1, Qt::AlignLeft);
//            row += 1; // to step into next col
//        }
//        else
//            dstypes_lay->addLayout(lay, row, col, 1, 1, Qt::AlignLeft);

//        // increment
//        if (row == 1)
//        {
//            row = 0;
//            ++col;
//        }
//        else
//        {
//            ++row;
//        }
//    }

    main_layout->addLayout(type_layout_);
    update();

    // associations

    QGridLayout* assoc_layout = new QGridLayout();

    QLabel* assoc_label = new QLabel("Associations");
    assoc_label->setFont(font_bold);
    assoc_layout->addWidget(assoc_label, 0, 0);

    associations_label_ = new QLabel();
    associations_label_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    assoc_layout->addWidget(associations_label_, 0, 1);

    main_layout->addLayout(assoc_layout);

    update();

    main_layout->addStretch();

    // limit stuff
    bool use_limit = dbo_manager_.useLimit();
    limit_check_ = new QCheckBox("Use Limit");
    limit_check_->setChecked(use_limit);
    connect(limit_check_, &QCheckBox::clicked, this, &DBObjectManagerLoadWidget::toggleUseLimit);
    main_layout->addWidget(limit_check_);


    QHBoxLayout* bottom_layout = new QHBoxLayout();

    limit_widget_ = new QWidget();

    QGridLayout* limit_layout = new QGridLayout();
    limit_layout->addWidget(new QLabel("Limit Min"), 0, 0);

    limit_min_edit_ = new QLineEdit();
    limit_min_edit_->setText(std::to_string(dbo_manager_.limitMin()).c_str());
    limit_min_edit_->setEnabled(use_limit);
    connect(limit_min_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMinChanged()));
    limit_layout->addWidget(limit_min_edit_, 0, 1);

    limit_layout->addWidget(new QLabel("Limit Max"), 1, 0);

    limit_max_edit_ = new QLineEdit();
    limit_max_edit_->setText(std::to_string(dbo_manager_.limitMax()).c_str());
    limit_max_edit_->setEnabled(use_limit);
    connect(limit_max_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMaxChanged()));
    limit_layout->addWidget(limit_max_edit_, 1, 1);

    limit_widget_->setLayout(limit_layout);

    if (!dbo_manager_.useLimit())
        limit_widget_->hide();

    bottom_layout->addWidget(limit_widget_);

    // load

    bottom_layout->addStretch();

    load_button_ = new QPushButton("Load");
    connect(load_button_, &QPushButton::clicked, this, &DBObjectManagerLoadWidget::loadButtonSlot);
    bottom_layout->addWidget(load_button_);

    main_layout->addLayout(bottom_layout);

    setLayout(main_layout);
}

DBObjectManagerLoadWidget::~DBObjectManagerLoadWidget() {}

void DBObjectManagerLoadWidget::toggleUseLimit()
{
    assert(limit_check_);
    assert(limit_widget_);
    assert(limit_min_edit_);
    assert(limit_max_edit_);

    bool checked = limit_check_->checkState() == Qt::Checked;
    logdbg << "DBObjectManagerLoadWidget: toggleUseLimit: setting use limit to " << checked;
    dbo_manager_.useLimit(checked);

    if (checked)
        limit_widget_->show();
    else
        limit_widget_->hide();

    limit_min_edit_->setEnabled(checked);
    limit_max_edit_->setEnabled(checked);
}

void DBObjectManagerLoadWidget::limitMinChanged()
{
    assert(limit_min_edit_);

    if (limit_min_edit_->text().size() == 0)
        return;

    unsigned int min = std::stoul(limit_min_edit_->text().toStdString());
    dbo_manager_.limitMin(min);
}
void DBObjectManagerLoadWidget::limitMaxChanged()
{
    assert(limit_max_edit_);

    if (limit_max_edit_->text().size() == 0)
        return;

    unsigned int max = std::stoul(limit_max_edit_->text().toStdString());
    dbo_manager_.limitMax(max);
}

void DBObjectManagerLoadWidget::loadButtonSlot()
{
    loginf << "DBObjectManagerLoadWidget: loadButtonSlot";

    if (COMPASS::instance().viewManager().getViews().size() == 0)
    {
        QMessageBox m_warning(QMessageBox::Warning, "Loading Not Possible",
                              "There are no Views active, so loading is not possible.",
                              QMessageBox::Ok);

        m_warning.exec();
        return;
    }

    assert(load_button_);

    if (loading_)
    {
        load_button_->setDisabled(true);
        dbo_manager_.quitLoading();
        return;
    }

    loading_ = true;
    load_button_->setText("Stop");

    dbo_manager_.loadSlot();
}

void DBObjectManagerLoadWidget::loadingDone()
{
    loading_ = false;
    load_button_->setText("Load");
    load_button_->setDisabled(false);
}

void DBObjectManagerLoadWidget::update()
{
    loginf << "DBObjectManagerLoadWidget: update: num data sources " << dbo_manager_.dataSources().size();

    // remove all previous
    while (QLayoutItem* item = type_layout_->takeAt(0))
    {
        assert(!item->layout()); // otherwise the layout will leak
        delete item->widget();
        delete item;
    }

    QFont font_bold;
    font_bold.setBold(true);

    unsigned int row = 0;
    unsigned int col_start = 0;
    unsigned int num_col_per_dstype = 5; // add 1 for spacing
    unsigned int dstype_col = 0;
    unsigned int dstyp_cnt = 0;

    for (auto& dstype_it : DBObjectManager::data_source_types_)
    {
        loginf << "DBObjectManagerLoadWidget: update: typ " << dstype_it << " cnt " << dstyp_cnt;

        if (dstype_it == "MLAT" || dstype_it == "Tracker")  // break into next column
        {
            row = 0;
            dstype_col++;
            col_start = dstype_col * num_col_per_dstype;
        }

        QCheckBox* dstyp_box = new QCheckBox(dstype_it.c_str());
        dstyp_box->setFont(font_bold);

        type_layout_->addWidget(dstyp_box, row, col_start, 1, num_col_per_dstype, Qt::AlignTop | Qt::AlignLeft);

        ++row;

        for (const auto& ds_it : dbo_manager_.dataSources())
        {
            //loginf << row << " '" << ds_it->dsType() << "' '" << dstype << "'";

            if (ds_it->dsType() != dstype_it)
                continue;

            QCheckBox* ds_box = new QCheckBox(ds_it->name().c_str());

            type_layout_->addWidget(ds_box, row, col_start, 1, num_col_per_dstype-1,
                                    Qt::AlignTop | Qt::AlignLeft);
            ++row;

            for (auto& cnt_it : ds_it->countsMap())
            {
                QCheckBox* dbcont_box = new QCheckBox(cnt_it.first.c_str());

                type_layout_->addWidget(dbcont_box, row, col_start+1,
                                        Qt::AlignTop | Qt::AlignRight);
                type_layout_->addWidget(new QLabel(QString::number(cnt_it.second)), row, col_start+2,
                                        Qt::AlignTop | Qt::AlignRight);
                type_layout_->addWidget(new QLabel(QString::number(0)), row, col_start+3,
                                        Qt::AlignTop | Qt::AlignRight);
                ++row;
            }
        }

        dstyp_cnt++;
    }


    //    QLayoutItem* item;
    //    while ((item = info_layout_->takeAt(0)) != nullptr)
    //    {
    //        info_layout_->removeItem(item);
    //    }

    //    for (auto& obj_it : object_manager_)
    //    {
    //        info_layout_->addWidget(obj_it.second->infoWidget());
    //    }

//    assert(associations_label_);
//    if (dbo_manager_.hasAssociations())
//    {
//        if (dbo_manager_.hasAssociationsDataSource())
//        {
//            std::string tmp = "From " + dbo_manager_.associationsDBObject() + ":" +
//                    dbo_manager_.associationsDataSourceName();
//            associations_label_->setText(tmp.c_str());
//        }
//        else
//            associations_label_->setText("All");
//    }
//    else
//        associations_label_->setText("None");
}
