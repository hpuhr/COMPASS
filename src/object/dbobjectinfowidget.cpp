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

#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QCheckBox>
#include <QTextEdit>

#include "dbobject.h"
#include "dbobjectinfowidget.h"
#include "dbovariable.h"
#include "logger.h"
#include "global.h"


DBObjectInfoWidget::DBObjectInfoWidget(DBObject &object, QWidget *parent, Qt::WindowFlags f)
    : QWidget (parent, f), object_(object), main_layout_(nullptr), main_check_(nullptr), status_label_(nullptr), loaded_count_label_(nullptr)
{
    QFont font_bold;
    font_bold.setBold(true);

    main_layout_ = new QGridLayout ();

    main_check_ = new QCheckBox (object.name().c_str());
    connect (main_check_, SIGNAL(toggled(bool)), this, SLOT(loadChangedSlot()));
    main_check_->setFont (font_bold);
    main_layout_->addWidget (main_check_);

    updateSlot();

    setLayout (main_layout_);
}

DBObjectInfoWidget::~DBObjectInfoWidget()
{

}

void DBObjectInfoWidget::loadChangedSlot()
{
    assert (main_check_);

    object_.loadingWanted(main_check_->checkState() == Qt::Checked);
    logdbg << "DBObjectInfoWidget: loadChangedSlot: wanted " << object_.loadingWanted();
}

void DBObjectInfoWidget::updateSlot()
{
    assert (main_check_);
    if (!object_.hasData())
    {
        main_check_->setEnabled(false);
        object_.loadingWanted(false);
        return;
    }

    if (!status_label_)
    {
        assert (main_layout_);
        assert (!loaded_count_label_);

        status_label_ = new QLabel (object_.status().c_str());
        status_label_->setAlignment(Qt::AlignRight);
        main_layout_->addWidget(status_label_, 0, 1);

        main_layout_->addWidget(new QLabel ("Loaded"), 1, 0);

        loaded_count_label_ = new QLabel ("?");
        loaded_count_label_->setAlignment(Qt::AlignRight);
        main_layout_->addWidget(loaded_count_label_, 1, 1);

        object_.loadingWanted(true);
    }

    assert (main_check_);
    assert (loaded_count_label_);

    main_check_->setEnabled(object_.hasData());
    main_check_->setChecked(object_.loadingWanted());
    status_label_->setText(object_.status().c_str());
    loaded_count_label_->setText(QString::number(object_.loadedCount())+" / "+QString::number(object_.count()));
}
