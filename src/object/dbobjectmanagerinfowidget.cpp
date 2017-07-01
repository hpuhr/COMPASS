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

#include "dbovariable.h"
#include "dbobject.h"
#include "dbobjectinfowidget.h"
#include "dbobjectmanagerinfowidget.h"
#include "dbobjectmanager.h"
#include "atsdb.h"
#include "global.h"

DBObjectManagerInfoWidget::DBObjectManagerInfoWidget(DBObjectManager &object_manager)
    : object_manager_(object_manager), info_layout_(nullptr), load_all_button_(nullptr)
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

    load_all_button_ = new QPushButton ("Load");
    connect (load_all_button_, SIGNAL(clicked()), this, SLOT(loadAllSlot()));
    main_layout->addWidget(load_all_button_);


    main_layout->addStretch();
    setLayout (main_layout);
}

DBObjectManagerInfoWidget::~DBObjectManagerInfoWidget()
{
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
