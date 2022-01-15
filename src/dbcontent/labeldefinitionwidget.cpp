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

#include "dbcontent/labeldefinitionwidget.h"

#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "dbovariable.h"

namespace dbContent
{

LabelDefinitionWidget::LabelDefinitionWidget(LabelDefinition* definition)
    : QWidget(), definition_(definition)  //, entries_ (definition->getEntries())
{
    assert(definition_);

    table_ = 0;

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout* layout = new QVBoxLayout();

    QLabel* label = new QLabel("Label Definition");
    label->setFont(font_bold);
    layout->addWidget(label);

    list_.append("Show");
    list_.append("Variable Name");
    list_.append("Prefix");
    list_.append("Suffix");

    // table_items_.resize (entries_.size(), std::vector<QTableWidgetItem*> (list_.size()));

    table_ = new QTableWidget();  // entries_.size(), list_.size()
    table_->setColumnCount(list_.size());
    table_->setHorizontalHeaderLabels(list_);

    table_->setEditTriggers(QAbstractItemView::DoubleClicked);
    table_->setColumnWidth(0, 60);
    table_->setColumnWidth(1, 350);
    table_->setColumnWidth(2, 100);
    table_->setColumnWidth(3, 100);

    setTable();

    layout->addWidget(table_);

    setLayout(layout);

    connect(table_, SIGNAL(cellChanged(int, int)), this, SLOT(cellChangedSlot(int, int)));

    setMinimumSize(QSize(800, 600));
    // target_->show();
}

LabelDefinitionWidget::~LabelDefinitionWidget() {}

void LabelDefinitionWidget::cellChangedSlot(int row, int column)
{
    logdbg << "DBOLabelDefinitionWidget: cellChangedSlot: row  " << row << " col " << column;

    QTableWidgetItem* item = table_->item(row, column);
    assert(item);

    const std::map<std::string, LabelEntry*>& entries = definition_->entries();

    assert(row < static_cast<int>(entries.size()));

    auto it = entries.begin();
    std::advance(it, row);
    std::string variable_name = it->first;
    LabelEntry& entry = definition_->entry(variable_name);

    if (column == 0)
    {
        entry.show(item->checkState() == Qt::Checked);
    }
    else if (column == 2)
    {
        entry.prefix(item->text().toStdString());
    }
    else if (column == 3)
    {
        entry.suffix(item->text().toStdString());
    }
    else
        throw std::runtime_error("DBOLabelDefinitionWidget: cellChangedSlot: wrong column");

    definition_->updateReadList();
}

void LabelDefinitionWidget::setTable()
{
    int row = 0;

    const std::map<std::string, LabelEntry*>& entries = definition_->entries();

    table_->setRowCount(entries.size());

    for (auto& it : entries)
    {
        for (int col = 0; col < list_.size(); col++)
        {
            QTableWidgetItem* newItem;

            if (col == 0)
            {
                newItem = new QTableWidgetItem();

                if (it.second->show())
                    newItem->setCheckState(Qt::Checked);
                else
                    newItem->setCheckState(Qt::Unchecked);
            }
            else if (col == 1)
            {
                newItem = new QTableWidgetItem(it.second->variableName().c_str());
                newItem->setFlags(Qt::ItemIsSelectable);
            }
            else if (col == 2)
            {
                newItem = new QTableWidgetItem(it.second->prefix().c_str());
            }
            else if (col == 3)
            {
                newItem = new QTableWidgetItem(it.second->suffix().c_str());
            }
            else
                newItem = new QTableWidgetItem();

            table_->setItem(row, col, newItem);
            // table_items_ [row] [col] = newItem;
        }
        row++;
    }
    table_->resizeColumnsToContents();
}

}
