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

#ifndef JSONDATAMAPPINGWIDGET_H
#define JSONDATAMAPPINGWIDGET_H

#include <QWidget>

class JSONDataMapping;

class JSONDataMappingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JSONDataMappingWidget(JSONDataMapping& mapping, QWidget *parent = nullptr);

    void setMapping (JSONDataMapping& mapping);

private:
    JSONDataMapping* mapping_ {nullptr};
};

#endif // JSONDATAMAPPINGWIDGET_H
