#include "datasourcetablemodel.h"
#include "datasourcemanager.h"
#include "logger.h"

DataSourceTableModel::DataSourceTableModel(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog)
    : ds_man_(ds_man), dialog_(dialog)
{

}

int DataSourceTableModel::rowCount(const QModelIndex& parent) const
{
    return ds_man_.dataSources().size();
}

int DataSourceTableModel::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QVariant DataSourceTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    {
        logdbg << "DataSourceTableModel: data: display role: row " << index.row() << " col " << index.column();

        assert (index.row() >= 0);
        assert (index.row() < ds_man_.dataSources().size());

        const std::unique_ptr<dbContent::DBDataSource>& ds = ds_man_.dataSources().at(index.row());

        logdbg << "DataSourceTableModel: data: got ds " << ds->id();

        assert (index.column() < table_columns_.size());
        std::string col_name = table_columns_.at(index.column()).toStdString();

        if (col_name == "Name")
            return ds->name().c_str();
        if (col_name == "Short Name")
        {
            if (ds->hasShortName())
                return ds->shortName().c_str();
            else
                return QVariant();
        }
        if (col_name == "DSType")
            return ds->dsType().c_str();
        if (col_name == "SAC")
            return ds->sac();
        if (col_name == "SIC")
            return ds->sic();
        else
            return QVariant();
    }
    default:
    {
        return QVariant();
    }
    }
}

QVariant DataSourceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex DataSourceTableModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex DataSourceTableModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags DataSourceTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

//    if (table_columns_.at(index.column()) == "comment")
//        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
//    else

    return QAbstractItemModel::flags(index);
}

unsigned int DataSourceTableModel::getIdOf (const QModelIndex& index)
{
    assert (index.isValid());

    assert (index.row() >= 0);
    assert (index.row() < ds_man_.dataSources().size());

    return ds_man_.dataSources().at(index.row())->id();
}
