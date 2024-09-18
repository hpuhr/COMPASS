#include "scatterseriesmodel.h"
#include "scatterseriestreeitem.h"
#include "logger.h"

using namespace std;

ScatterSeriesModel::ScatterSeriesModel()
: QAbstractItemModel(nullptr)
{

}

ScatterSeriesModel::~ScatterSeriesModel()
{

}

QVariant ScatterSeriesModel::data(const QModelIndex& index, int role) const
{
    logdbg << "ScatterSeriesModel: data: index row " << index.row() << " col " << index.column()
           << " valid " << index.isValid();

    if (!index.isValid())
        return QVariant();

    // switch (role)
    // {
    // case IconRole:
    // {
    //     if (index.column() == 1)  // only col 0 have icons
    //         return QVariant();

    //     logdbg << "ScatterSeriesModel: data: icon role";
    //     OSGLayerTreeItem* item = static_cast<OSGLayerTreeItem*>(index.internalPointer());
    //     return item->icon();
    // }
    // case Qt::DisplayRole:
    // {
    //     logdbg << "ScatterSeriesModel: data: display role";
    //     OSGLayerTreeItem* item = static_cast<OSGLayerTreeItem*>(index.internalPointer());
    //     return item->data(index.column());
    // }
    // //    case Qt::BackgroundRole:
    // //    {
    // //        return QVariant(QColor(Qt::black));
    // //    }
    // default:
    // {
    //     logdbg << "ScatterSeriesModel: data: default";
    //     return QVariant();
    // }
    // }
}
Qt::ItemFlags ScatterSeriesModel::flags(const QModelIndex& index) const
{

}
QVariant ScatterSeriesModel::headerData(int section, Qt::Orientation orientation,
                    int role) const
{

}
QModelIndex ScatterSeriesModel::index(int row, int column,
                  const QModelIndex& parent) const
{

}
QModelIndex ScatterSeriesModel::parent(const QModelIndex& index) const
{

}
int ScatterSeriesModel::rowCount(const QModelIndex& parent) const
{

}
int ScatterSeriesModel::columnCount(const QModelIndex& parent) const
{
    logdbg << "ScatterSeriesModel: columnCount: index valid " << parent.isValid();

    return 2;
}

ScatterSeriesCollection ScatterSeriesModel::scatterSeries() const
{
    return scatter_series_;
}
