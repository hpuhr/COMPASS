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
#include <QGridLayout>
#include <QLabel>

#include "jsondatamappingwidget.h"

JSONDataMappingWidget::JSONDataMappingWidget(JSONDataMapping& mapping, QWidget *parent)
    : QWidget(parent), mapping_(&mapping)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("JSON Mapping ");
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    setLayout (main_layout);

    show();
}

void JSONDataMappingWidget::setMapping (JSONDataMapping& mapping)
{
    mapping_ = &mapping;
}
