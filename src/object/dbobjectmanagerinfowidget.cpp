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

#include "dbovariable.h"
#include "dbobject.h"
#include "dbobjectinfowidget.h"
#include "dbobjectmanagerinfowidget.h"
#include "dbobjectmanager.h"
#include "atsdb.h"
#include "global.h"
#include "stringconv.h"

using Utils::String;

DBObjectManagerInfoWidget::DBObjectManagerInfoWidget(DBObjectManager &object_manager)
    : object_manager_(object_manager), info_layout_(nullptr), limit_check_(nullptr), limit_min_edit_ (nullptr), limit_max_edit_(nullptr), load_all_button_(nullptr)
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

    // limit stuff
    bool use_limit = object_manager_.useLimit();
    limit_check_ = new QCheckBox ("Use Limit");
    limit_check_->setChecked(use_limit);
    connect (limit_check_, SIGNAL(toggled(bool)), this, SLOT(toggleUseLimit()));
    main_layout->addWidget(limit_check_);

    QGridLayout *limit_layout = new QGridLayout ();

    limit_layout->addWidget(new QLabel ("Limit Min"), 0, 0);

    limit_min_edit_ = new QLineEdit ();
    limit_min_edit_->setText (String::intToString(object_manager_.limitMin()).c_str());
    limit_min_edit_->setEnabled(use_limit);
    connect( limit_min_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMinChanged()) );
    limit_layout->addWidget(limit_min_edit_, 0, 1);

    limit_layout->addWidget(new QLabel ("Limit Max"), 1, 0);

    limit_max_edit_ = new QLineEdit ();
    limit_max_edit_->setText (String::intToString(object_manager_.limitMax()).c_str());
    limit_max_edit_->setEnabled(use_limit);
    connect( limit_max_edit_, SIGNAL(textChanged(QString)), this, SLOT(limitMaxChanged()) );
    limit_layout->addWidget(limit_max_edit_, 1, 1);

    main_layout->addLayout(limit_layout);

    // load
    load_all_button_ = new QPushButton ("Load");
    connect (load_all_button_, SIGNAL(clicked()), this, SLOT(loadAllSlot()));
    main_layout->addWidget(load_all_button_);


    main_layout->addStretch();
    setLayout (main_layout);
}

DBObjectManagerInfoWidget::~DBObjectManagerInfoWidget()
{
}

void DBObjectManagerInfoWidget::toggleUseLimit()
{
    assert (limit_check_);
    assert (limit_min_edit_);
    assert (limit_max_edit_);

    bool checked = limit_check_->checkState() == Qt::Checked;
    logdbg  << "DBObjectManagerInfoWidget: toggleUseLimit: setting use limit to " << checked;
    object_manager_.useLimit(checked);

    limit_min_edit_->setEnabled(checked);
    limit_max_edit_->setEnabled(checked);
}


void DBObjectManagerInfoWidget::limitMinChanged()
{
  assert (limit_min_edit_);

  if (limit_min_edit_->text().size() == 0)
      return;

  unsigned int min = String::intFromString (limit_min_edit_->text().toStdString());
  object_manager_.limitMin(min);
}
void DBObjectManagerInfoWidget::limitMaxChanged()
{
  assert (limit_max_edit_);

  if (limit_max_edit_->text().size() == 0)
      return;

  unsigned int max = String::intFromString (limit_max_edit_->text().toStdString());
  object_manager_.limitMax(max);
}

void DBObjectManagerInfoWidget::loadAllSlot ()
{
    object_manager_.loadSlot();
}

void DBObjectManagerInfoWidget::updateSlot ()
{
    QLayoutItem* item;
    while ((item = info_layout_->takeAt(0)) != nullptr)
    {
        info_layout_->removeItem(item);
    }

    for (auto object : object_manager_.objects())
    {
        info_layout_->addWidget(object.second->infoWidget());
    }
}
