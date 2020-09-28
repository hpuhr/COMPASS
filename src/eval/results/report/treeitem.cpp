#include "eval/results/report/treeitem.h"

namespace EvaluationResultsReport
{
    TreeItem::TreeItem(TreeItem* parent_item)
        : parent_item_(parent_item)
    {

    }

    TreeItem* TreeItem::parentItem()
    {
        return parent_item_;
    }
}

