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

#include "dbodatasourcedefinitionwidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "compass.h"
#include "dbobject.h"
#include "dbodatasource.h"
#include "dbovariable.h"

DBODataSourceDefinitionWidget::DBODataSourceDefinitionWidget(DBObject& object,
                                                             DBODataSourceDefinition& definition,
                                                             QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      object_(object),
      definition_(definition)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel("Edit data source definition");
    main_label->setFont(font_big);
    main_layout->addWidget(main_label);

    setLayout(main_layout);
}

DBODataSourceDefinitionWidget::~DBODataSourceDefinitionWidget() {}



//void DBODataSourceDefinitionWidget::updateVariableSelectionBox(QComboBox* box,
//                                                               const std::string& meta_table_name,
//                                                               const std::string& value,
//                                                               bool empty_allowed)
//{
//    logdbg << "DBODataSourceDefinitionWidget: updateVariableSelectionBox: value " << value;
//    assert(box);

//    while (box->count() > 0)
//        box->removeItem(0);

//    DBSchema& schema = schema_manager_.getCurrentSchema();

//    assert(schema.hasMetaTable(meta_table_name));

//    const MetaDBTable& meta_table = schema.metaTable(meta_table_name);

//    auto table_columns = meta_table.columns();

//    if (empty_allowed)
//        box->addItem("");

//    for (auto it = table_columns.begin(); it != table_columns.end(); it++)
//        box->addItem(it->first.c_str());

//    box->setCurrentText(value.c_str());
//}
