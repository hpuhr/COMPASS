#include "evaluationstandardtreeitem.h"

EvaluationStandardTreeItem::EvaluationStandardTreeItem(EvaluationStandardTreeItem* parent_item)
    : parent_item_(parent_item)
{
}

EvaluationStandardTreeItem* EvaluationStandardTreeItem::parentItem()
{
    return parent_item_;
}
