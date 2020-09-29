#include "eval/results/report/treeitem.h"

namespace EvaluationResultsReport
{
    TreeItem::TreeItem(const string name, TreeItem* parent_item)
        : name_(name), parent_item_(parent_item)
    {

    }

    TreeItem* TreeItem::parentItem()
    {
        return parent_item_;
    }

    string TreeItem::name() const
    {
        return name_;
    }
}

