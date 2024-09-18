#pragma once

#include "scatterseries.h"
#include "scatterseriestreeitem.h"


#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>


class ScatterSeriesModel : public QAbstractItemModel
{
public:
    ScatterSeriesModel();
    virtual ~ScatterSeriesModel();

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    ScatterSeriesCollection scatterSeries() const;

    enum DataRole
    {
        IconRole = Qt::UserRole + 100
    };

private:
    ScatterSeriesCollection scatter_series_;

    std::unique_ptr<ScatterSeriesTreeItem> root_item_;
};

