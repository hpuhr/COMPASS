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

using namespace Utils::String;

DBObjectManagerLoadWidget::DBObjectManagerLoadWidget(DBObjectManager& object_manager)
    : object_manager_(object_manager)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel("Database Objects");
    main_label->setFont(font_bold);
    main_layout->addWidget(main_label);

    // data sources, per type



    // associations

    QGridLayout* assoc_layout = new QGridLayout();

    QLabel* assoc_label = new QLabel("Associations");
    assoc_label->setFont(font_bold);
    assoc_layout->addWidget(assoc_label, 0, 0);

    associations_label_ = new QLabel();
    associations_label_->setAlignment(Qt::AlignRight);
    assoc_layout->addWidget(associations_label_, 0, 1);

    main_layout->addLayout(assoc_layout);

    update();

    main_layout->addStretch();

    // limit stuff
    bool use_limit = object_manager_.useLimit();
    limit_check_ = new QCheckBox("Use Limit");
    limit_check_->setChecked(use_limit);
    connect(limit_check_, &QCheckBox::clicked, this, &DBObjectManagerLoadWidget::toggleUseLimit);
    main_layout->addWidget(limit_check_);


    QHBoxLayout* bottom_layout = new QHBoxLayout();

    limit_widget_ = new QWidget();

    QGridLayout* limit_layout = new QGridLayout();
    limit_layout->addWidget(new QLabel("Limit Min"), 0, 0);

    limit_min_edit_ = new QLineEdit();
    limit_min_edit_->setText(std::to_string(object_manager_.limitMin()).c_str());
    limit_min_edit_->setEnabled(use_limit);
    connect(limit_min_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMinChanged()));
    limit_layout->addWidget(limit_min_edit_, 0, 1);

    limit_layout->addWidget(new QLabel("Limit Max"), 1, 0);

    limit_max_edit_ = new QLineEdit();
    limit_max_edit_->setText(std::to_string(object_manager_.limitMax()).c_str());
    limit_max_edit_->setEnabled(use_limit);
    connect(limit_max_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMaxChanged()));
    limit_layout->addWidget(limit_max_edit_, 1, 1);

    limit_widget_->setLayout(limit_layout);

    if (!object_manager_.useLimit())
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
    object_manager_.useLimit(checked);

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
    object_manager_.limitMin(min);
}
void DBObjectManagerLoadWidget::limitMaxChanged()
{
    assert(limit_max_edit_);

    if (limit_max_edit_->text().size() == 0)
        return;

    unsigned int max = std::stoul(limit_max_edit_->text().toStdString());
    object_manager_.limitMax(max);
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
        object_manager_.quitLoading();
        return;
    }

    loading_ = true;
    load_button_->setText("Stop");

    object_manager_.loadSlot();
}

void DBObjectManagerLoadWidget::loadingDone()
{
    loading_ = false;
    load_button_->setText("Load");
    load_button_->setDisabled(false);
}

void DBObjectManagerLoadWidget::update()
{
//    QLayoutItem* item;
//    while ((item = info_layout_->takeAt(0)) != nullptr)
//    {
//        info_layout_->removeItem(item);
//    }

//    for (auto& obj_it : object_manager_)
//    {
//        info_layout_->addWidget(obj_it.second->infoWidget());
//    }

    assert(associations_label_);
    if (object_manager_.hasAssociations())
    {
        if (object_manager_.hasAssociationsDataSource())
        {
            std::string tmp = "From " + object_manager_.associationsDBObject() + ":" +
                              object_manager_.associationsDataSourceName();
            associations_label_->setText(tmp.c_str());
        }
        else
            associations_label_->setText("All");
    }
    else
        associations_label_->setText("None");
}
