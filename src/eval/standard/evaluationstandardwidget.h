#ifndef EVALUATIONSTANDARDWIDGET_H
#define EVALUATIONSTANDARDWIDGET_H

#include <QWidget>
#include <QModelIndex>
#include <QTreeView>

#include <memory>

#include "evaluationstandardtreemodel.h"

class EvaluationStandard;

class QTreeView;

class EvaluationStandardWidget : public QWidget
{
    Q_OBJECT

public slots:
    void itemClickedSlot(const QModelIndex& index);

public:
    EvaluationStandardWidget(EvaluationStandard& standard);

protected:
    EvaluationStandard& standard_;

    EvaluationStandardTreeModel standard_model_;
    std::unique_ptr<QTreeView> tree_view_;
};

#endif // EVALUATIONSTANDARDWIDGET_H
