#include "datasourcetablemodel.h"
#include "datasourcemanager.h"
#include "logger.h"
#include "files.h"

using namespace Utils;
using namespace std;

DataSourceTableModel::DataSourceTableModel(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog)
    : ds_man_(ds_man), dialog_(dialog)
{
    db_icon_ = QIcon(Files::getIconFilepath("db.png").c_str());
    config_icon_ = QIcon(Files::getIconFilepath("configuration.png").c_str());
}

int DataSourceTableModel::rowCount(const QModelIndex& parent) const
{
    return ds_man_.getAllDsIDs().size();
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
        assert (index.row() < ds_man_.getAllDsIDs().size());

        unsigned int ds_id = ds_man_.getAllDsIDs().at(index.row());

        logdbg << "DataSourceTableModel: data: got ds_id " << ds_id;

        assert (index.column() < table_columns_.size());
        std::string col_name = table_columns_.at(index.column()).toStdString();

        if (ds_man_.hasDBDataSource(ds_id))
        {
            dbContent::DBDataSource& ds = ds_man_.dbDataSource(ds_id);
            assert (ds_man_.hasConfigDataSource(ds_id));

            if (col_name == "Name")
                return ds.name().c_str();
            if (col_name == "Short Name")
            {
                if (ds.hasShortName())
                    return ds.shortName().c_str();
                else
                    return QVariant();
            }
            if (col_name == "DSType")
                return ds.dsType().c_str();
            if (col_name == "SAC")
                return ds.sac();
            if (col_name == "SIC")
                return ds.sic();
            else
                return QVariant();
        }
        else
        {
            assert (ds_man_.hasConfigDataSource(ds_id));

            dbContent::ConfigurationDataSource& ds = ds_man_.configDataSource(ds_id);

            if (col_name == "Name")
                return ds.name().c_str();
            if (col_name == "Short Name")
            {
                if (ds.hasShortName())
                    return ds.shortName().c_str();
                else
                    return QVariant();
            }
            if (col_name == "DSType")
                return ds.dsType().c_str();
            if (col_name == "SAC")
                return ds.sac();
            if (col_name == "SIC")
                return ds.sic();
            else
                return QVariant();
        }
    }
    case Qt::DecorationRole:
    {
        assert (index.row() >= 0);
        assert (index.row() < ds_man_.getAllDsIDs().size());

        unsigned int ds_id = ds_man_.getAllDsIDs().at(index.row());

        logdbg << "DataSourceTableModel: data: got ds_id " << ds_id;

        assert (index.column() < table_columns_.size());
        std::string col_name = table_columns_.at(index.column()).toStdString();

        if (col_name != "In DB" && col_name != "In CfG")
            return QVariant();

        if (col_name == "In DB" && ds_man_.hasDBDataSource(ds_id))
            return db_icon_;
        else if (col_name == "In CfG" && ds_man_.hasConfigDataSource(ds_id))
            return config_icon_;
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
    assert (index.row() < ds_man_.getAllDsIDs().size());

    return ds_man_.getAllDsIDs().at(index.row());
}
