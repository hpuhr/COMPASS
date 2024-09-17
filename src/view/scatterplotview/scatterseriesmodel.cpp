#include "scatterseriesmodel.h"

ScatterSeriesModel::ScatterSeriesModel() {}

ScatterSeriesModel::~ScatterSeriesModel()
{

}

QVariant ScatterSeriesModel::data(const QModelIndex& index, int role) const
{

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

}

ScatterSeriesCollection ScatterSeriesModel::scatterSeries() const
{
    return scatter_series_;
}
