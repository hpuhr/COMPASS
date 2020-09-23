#include "evaluationstandardrootitem.h"
#include "evaluationstandard.h"

EvaluationStandardRootItem::EvaluationStandardRootItem(EvaluationStandard &standard)
  : EvaluationStandardTreeItem(nullptr), standard_(standard)
{

}

EvaluationStandardTreeItem* EvaluationStandardRootItem::child(int row)
{
    if (row < 0 || row > 0)
        return nullptr;

    return &standard_;
}

int EvaluationStandardRootItem::childCount() const
{
    return 1;
}

int EvaluationStandardRootItem::columnCount() const
{
    return 1;
}

QVariant EvaluationStandardRootItem::data(int column) const
{
    assert (column == 0);

    return "Standard";
}

int EvaluationStandardRootItem::row() const
{
    return 0;
}
