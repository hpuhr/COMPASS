#ifndef EVALUATIONSTANDARDTREEITEM_H
#define EVALUATIONSTANDARDTREEITEM_H

#include <QVariant>

class EvaluationStandardTreeItem
{
public:
    EvaluationStandardTreeItem(EvaluationStandardTreeItem* parent_item = nullptr);

    //void appendChild(EvaluationStandardTreeItem *child);

    virtual EvaluationStandardTreeItem *child(int row) = 0;
    virtual int childCount() const = 0;
    virtual int columnCount() const = 0;
    virtual QVariant data(int column) const = 0;
    virtual int row() const = 0;
    EvaluationStandardTreeItem* parentItem();

protected:
    EvaluationStandardTreeItem* parent_item_ {nullptr};
};

#endif // EVALUATIONSTANDARDTREEITEM_H
