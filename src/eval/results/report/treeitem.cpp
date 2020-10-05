#include "eval/results/report/treeitem.h"
#include "logger.h"

namespace EvaluationResultsReport
{
    TreeItem::TreeItem(const string name, TreeItem* parent_item)
        : name_(name), parent_item_(parent_item)
    {
        if (parent_item_)
            id_ = parent_item_->id()+":"+name_;
        else
            id_ = name_;

        logdbg << "TreeItem: constructor: id '" << id_ << "'";
    }

    TreeItem* TreeItem::parentItem()
    {
        return parent_item_;
    }

    string TreeItem::name() const
    {
        return name_;
    }
    
    string TreeItem::id() const
    {
        return id_;
    }
}

