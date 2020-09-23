#ifndef EVALUATIONSTANDARDTREEMODEL_H
#define EVALUATIONSTANDARDTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class EvaluationStandard;
class EvaluationStandardTreeItem;


class EvaluationStandardTreeModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit EvaluationStandardTreeModel(EvaluationStandard& standard, QObject* parent = nullptr);
    ~EvaluationStandardTreeModel();

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    EvaluationStandard& standard_;

    EvaluationStandardTreeItem* root_item;
};

#endif // EVALUATIONSTANDARDTREEMODEL_H
