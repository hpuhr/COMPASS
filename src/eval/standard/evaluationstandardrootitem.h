#ifndef EVALUATIONSTANDARDROOTITEM_H
#define EVALUATIONSTANDARDROOTITEM_H

#include "evaluationstandardtreeitem.h"

class EvaluationStandard;

class EvaluationStandardRootItem : public EvaluationStandardTreeItem
{
public:
    EvaluationStandardRootItem(EvaluationStandard &standard);

    virtual EvaluationStandardTreeItem *child(int row) override;
    virtual int childCount() const override;
    virtual int columnCount() const override;
    virtual QVariant data(int column) const override;
    virtual int row() const override;

protected:
    EvaluationStandard& standard_;
};

#endif // EVALUATIONSTANDARDROOTITEM_H
