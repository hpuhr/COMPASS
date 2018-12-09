/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>

#include "dbovariable.h"
#include "dbobject.h"
#include "dbobjectinfowidget.h"
#include "dbobjectmanagerloadwidget.h"
#include "dbobjectmanager.h"
#include "dbovariableselectionwidget.h"
#include "atsdb.h"
#include "viewmanager.h"
#include "global.h"
#include "stringconv.h"

using namespace Utils::String;

DBObjectManagerLoadWidget::DBObjectManagerLoadWidget(DBObjectManager &object_manager)
    : object_manager_(object_manager), info_layout_(nullptr), filters_check_(nullptr), order_check_(nullptr),order_ascending_check_(nullptr),
      order_variable_widget_(nullptr), limit_check_(nullptr), limit_min_edit_ (nullptr), limit_max_edit_(nullptr), load_button_(nullptr)
{
    unsigned int frame_width = FRAME_SIZE;

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Database Objects");
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    info_layout_ = new QVBoxLayout ();
    updateSlot();
    main_layout->addLayout(info_layout_);

    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine); // Horizontal line
    line->setFrameShadow(QFrame::Sunken);
    line->setLineWidth(1);
    main_layout->addWidget(line);

    // use filters stuff
    filters_check_ = new QCheckBox ("Use Filters");
    filters_check_->setChecked(object_manager_.useFilters());
    connect (filters_check_, SIGNAL(toggled(bool)), this, SLOT(toggleUseFilters()));
    main_layout->addWidget(filters_check_);

    QFrame *line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine); // Horizontal line
    line2->setFrameShadow(QFrame::Sunken);
    line2->setLineWidth(1);
    main_layout->addWidget(line2);

    // there is to be order!
    order_check_ = new QCheckBox("Use Order");
    order_check_->setChecked(object_manager.useOrder());
    connect(order_check_, SIGNAL( clicked() ), this, SLOT( toggleUseOrder() ));
    main_layout->addWidget(order_check_);

    order_ascending_check_= new QCheckBox("Order Ascending");
    order_ascending_check_->setChecked(object_manager.useOrderAscending());
    connect(order_ascending_check_, SIGNAL( clicked() ), this, SLOT( toggleOrderAscending() ));
    main_layout->addWidget(order_ascending_check_);

    main_layout->addWidget (new QLabel ("Order Variable:"));
    order_variable_widget_ = new DBOVariableSelectionWidget (true, this);
    order_variable_widget_->showMetaVariables(true);

    if (object_manager.hasOrderVariable())
        order_variable_widget_->selectedVariable(object_manager.orderVariable());
    if (object_manager.hasOrderMetaVariable())
        order_variable_widget_->selectedMetaVariable(object_manager.orderMetaVariable());

    connect (order_variable_widget_, SIGNAL (selectionChanged()), this, SLOT(orderVariableChanged()));
    main_layout->addWidget (order_variable_widget_);

    QFrame *line3 = new QFrame(this);
    line3->setFrameShape(QFrame::HLine); // Horizontal line
    line3->setFrameShadow(QFrame::Sunken);
    line3->setLineWidth(1);
    main_layout->addWidget(line3);

    // limit stuff
    bool use_limit = object_manager_.useLimit();
    limit_check_ = new QCheckBox ("Use Limit");
    limit_check_->setChecked(use_limit);
    connect (limit_check_, SIGNAL(toggled(bool)), this, SLOT(toggleUseLimit()));
    main_layout->addWidget(limit_check_);

    QGridLayout *limit_layout = new QGridLayout ();
    limit_layout->addWidget(new QLabel ("Limit Min"), 0, 0);

    limit_min_edit_ = new QLineEdit ();
    limit_min_edit_->setText (std::to_string(object_manager_.limitMin()).c_str());
    limit_min_edit_->setEnabled(use_limit);
    connect( limit_min_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMinChanged()) );
    limit_layout->addWidget(limit_min_edit_, 0, 1);

    limit_layout->addWidget(new QLabel ("Limit Max"), 1, 0);

    limit_max_edit_ = new QLineEdit ();
    limit_max_edit_->setText (std::to_string(object_manager_.limitMax()).c_str());
    limit_max_edit_->setEnabled(use_limit);
    connect( limit_max_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMaxChanged()) );
    limit_layout->addWidget(limit_max_edit_, 1, 1);

    main_layout->addLayout(limit_layout);
    main_layout->addStretch();

    // load
    load_button_ = new QPushButton ("Load");
    connect (load_button_, SIGNAL(clicked()), this, SLOT(loadButtonSlot()));
    main_layout->addWidget(load_button_);

    setLayout (main_layout);
}

DBObjectManagerLoadWidget::~DBObjectManagerLoadWidget()
{
}

void DBObjectManagerLoadWidget::toggleUseFilters()
{
    assert (filters_check_);

    bool checked = filters_check_->checkState() == Qt::Checked;
    logdbg  << "DBObjectManagerLoadWidget: toggleUseFilters: setting use limit to " << checked;
    object_manager_.useFilters(checked);
}

void DBObjectManagerLoadWidget::toggleUseOrder ()
{
    assert (order_check_);
    bool checked = order_check_->checkState() == Qt::Checked;
    object_manager_.useOrder(checked);
}

void DBObjectManagerLoadWidget::toggleOrderAscending ()
{
    assert (order_ascending_check_);
    bool checked = order_ascending_check_->checkState() == Qt::Checked;
    object_manager_.useOrderAscending(checked);
}

void DBObjectManagerLoadWidget::orderVariableChanged ()
{
    assert (order_variable_widget_);

    if (order_variable_widget_->hasVariable())
        object_manager_.orderVariable(order_variable_widget_->selectedVariable());
    else if (order_variable_widget_->hasMetaVariable())
        object_manager_.orderMetaVariable(order_variable_widget_->selectedMetaVariable());
    else
        object_manager_.clearOrderVariable();
}


void DBObjectManagerLoadWidget::toggleUseLimit()
{
    assert (limit_check_);
    assert (limit_min_edit_);
    assert (limit_max_edit_);

    bool checked = limit_check_->checkState() == Qt::Checked;
    logdbg  << "DBObjectManagerLoadWidget: toggleUseLimit: setting use limit to " << checked;
    object_manager_.useLimit(checked);

    limit_min_edit_->setEnabled(checked);
    limit_max_edit_->setEnabled(checked);
}


void DBObjectManagerLoadWidget::limitMinChanged()
{
    assert (limit_min_edit_);

    if (limit_min_edit_->text().size() == 0)
        return;

    unsigned int min = std::stoul (limit_min_edit_->text().toStdString());
    object_manager_.limitMin(min);
}
void DBObjectManagerLoadWidget::limitMaxChanged()
{
    assert (limit_max_edit_);

    if (limit_max_edit_->text().size() == 0)
        return;

    unsigned int max = std::stoul (limit_max_edit_->text().toStdString());
    object_manager_.limitMax(max);
}

void DBObjectManagerLoadWidget::loadButtonSlot ()
{
    loginf << "DBObjectManagerLoadWidget: loadButtonSlot";

    if (ATSDB::instance().viewManager().getViews().size() == 0)
    {
        QMessageBox m_warning (QMessageBox::Warning, "Loading Not Possible",
                               "There are no Views active, so loading is not possible.",
                               QMessageBox::Ok);

        m_warning.exec();
        return;
    }

    assert (load_button_);

    if (loading_)
    {
        load_button_->setDisabled (true);
        object_manager_.quitLoading ();
        return;
    }

    loading_ = true;
    load_button_->setText("Stop");

    object_manager_.loadSlot();
}

void DBObjectManagerLoadWidget::loadingDone ()
{
    loading_ = false;
    load_button_->setText("Load");
    load_button_->setDisabled (false);
}

void DBObjectManagerLoadWidget::updateSlot ()
{
    QLayoutItem* item;
    while ((item = info_layout_->takeAt(0)) != nullptr)
    {
        info_layout_->removeItem(item);
    }

    for (auto& obj_it : object_manager_)
    {
        info_layout_->addWidget(obj_it.second->infoWidget());
    }
}
