#ifndef EVALUATIONRESULTSREPORTTREEITEM_H
#define EVALUATIONRESULTSREPORTTREEITEM_H

#include <QVariant>

#include <memory>

namespace EvaluationResultsReport
{
    class TreeItem
    {
    public:
        TreeItem(TreeItem* parent_item);

        virtual TreeItem *child(int row) = 0;
        virtual int childCount() const = 0;
        virtual int columnCount() const = 0;
        virtual QVariant data(int column) const = 0;
        virtual int row() const = 0;
        TreeItem* parentItem();

    protected:
        TreeItem* parent_item_ {nullptr};
    };
}

#endif // EVALUATIONRESULTSREPORTTREEITEM_H
