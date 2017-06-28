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
    : QWidget (parent, f), object_(object), main_layout_(nullptr), main_label_(nullptr), status_label_(nullptr), total_count_label_(nullptr), loaded_count_label_(nullptr),
      load_check_(nullptr), load_button_(nullptr), loading_wanted_(true)
{
    QFont font_bold;
    font_bold.setBold(true);

    main_layout_ = new QVBoxLayout ();

    main_label_ = new QLabel (object.name().c_str());
    main_label_->setFont (font_bold);
    main_layout_->addWidget (main_label_);

    updateSlot();

    setLayout (main_layout_);
}

DBObjectInfoWidget::~DBObjectInfoWidget()
{

}

void DBObjectInfoWidget::loadChangedSlot()
{
    assert (load_check_);
    assert (load_button_);

    loading_wanted_ = load_check_->checkState() == Qt::Checked;
    load_button_->setEnabled(loading_wanted_);
}

void DBObjectInfoWidget::loadSlot()
{
    object_.load();
}

void DBObjectInfoWidget::updateSlot()
{
    assert (main_label_);
    if (!object_.hasData())
    {
        main_label_->setEnabled(false);
        return;
    }

    main_label_->setEnabled(true);

    if (!load_check_)
    {
        assert (main_layout_);
        assert (!load_button_);

        QGridLayout *grid = new QGridLayout ();

        grid->addWidget(new QLabel ("Total"), 0, 0);

        total_count_label_ = new QLabel ("?");
        grid->addWidget(total_count_label_, 0, 1);


        grid->addWidget(new QLabel ("Loaded"), 1, 0);

        loaded_count_label_ = new QLabel ("?");
        grid->addWidget(loaded_count_label_, 1, 1);

        grid->addWidget(new QLabel ("Status"), 2, 0);

        status_label_ = new QLabel (object_.status().c_str());
        grid->addWidget(status_label_, 2, 1);

        load_check_ = new QCheckBox ("Load");
        load_check_->setChecked(loading_wanted_);
        connect(load_check_, SIGNAL(toggled(bool)), this, SLOT(loadChangedSlot()));
        grid->addWidget(load_check_, 3, 0);

        QPixmap load_pixmap("./data/icons/load.png");
        QIcon load_icon(load_pixmap);

        load_button_ = new QPushButton ();
        load_button_->setIcon(load_icon);
        load_button_->setFixedSize ( UI_ICON_SIZE );
        //load_button_->setFlat(true);
        connect(load_button_, SIGNAL( clicked() ), this, SLOT(loadSlot()));
        grid->addWidget (load_button_, 3, 1);

        main_layout_->addLayout (grid);
    }

    assert (total_count_label_);
    assert (loaded_count_label_);
    assert (load_check_);
    assert (load_button_);

    total_count_label_->setText(QString::number(object_.count()));
    loaded_count_label_->setText(QString::number(object_.loadedCount()));
    status_label_->setText(object_.status().c_str());
    load_check_->setEnabled(object_.hasData());
    load_button_->setEnabled(object_.hasData());
}
