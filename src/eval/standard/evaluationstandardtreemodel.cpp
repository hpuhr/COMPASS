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

#include "evaluationstandardtreemodel.h"
#include "evaluationstandard.h"

EvaluationStandardTreeModel::EvaluationStandardTreeModel(EvaluationStandard& standard, QObject* parent)
    : QAbstractItemModel(parent), standard_(standard)
{
    root_item_ = &standard.rootItem();
}

EvaluationStandardTreeModel::~EvaluationStandardTreeModel()
{
}

int EvaluationStandardTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<EvaluationStandardTreeItem*>(parent.internalPointer())->columnCount();
    return root_item_->columnCount();
}

QVariant EvaluationStandardTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    EvaluationStandardTreeItem *item = static_cast<EvaluationStandardTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags EvaluationStandardTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant EvaluationStandardTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return root_item_->data(section);

    return QVariant();
}

QModelIndex EvaluationStandardTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    EvaluationStandardTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = root_item_;
    else
        parentItem = static_cast<EvaluationStandardTreeItem*>(parent.internalPointer());

    EvaluationStandardTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex EvaluationStandardTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    EvaluationStandardTreeItem *childItem = static_cast<EvaluationStandardTreeItem*>(index.internalPointer());
    EvaluationStandardTreeItem *parentItem = childItem->parentItem();

    if (parentItem == root_item_)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int EvaluationStandardTreeModel::rowCount(const QModelIndex &parent) const
{
    EvaluationStandardTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = root_item_;
    else
        parentItem = static_cast<EvaluationStandardTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void EvaluationStandardTreeModel::beginReset()
{
    beginResetModel();
}
void EvaluationStandardTreeModel::endReset()
{
    endResetModel();
}


