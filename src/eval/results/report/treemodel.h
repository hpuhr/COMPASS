#ifndef EVALUATIONRESULTSREPORTTREEMODEL_H
#define EVALUATIONRESULTSREPORTTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <memory>

namespace EvaluationResultsReport
{
    using namespace std;

    class RootItem;

    class TreeModel : public QAbstractItemModel
    {
    public:
        TreeModel();

        QVariant data(const QModelIndex& index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column,
                          const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        void beginReset();
        void endReset();

        shared_ptr<RootItem> rootItem() const;

    protected:
        shared_ptr<RootItem> root_item_;
    };

}

#endif // EVALUATIONRESULTSREPORTTREEMODEL_H
