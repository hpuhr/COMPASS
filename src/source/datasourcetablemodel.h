#ifndef DATASOURCETABLEMODEL_H
#define DATASOURCETABLEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

class DataSourceManager;
class DataSourcesConfigurationDialog;

class DataSourceTableModel : public QAbstractItemModel
{
public:
    DataSourceTableModel(DataSourceManager& ds_man, DataSourcesConfigurationDialog& dialog);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    //bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    unsigned int getIdOf (const QModelIndex& index);

    void updateDataSource(unsigned int ds_id);

protected:
    DataSourceManager& ds_man_;
    DataSourcesConfigurationDialog& dialog_;

    QStringList table_columns_ {"Name", "Short Name", "DSType", "SAC", "SIC", "In DB", "In Cfg"};

    QIcon db_icon_;
    QIcon config_icon_;
};

#endif // DATASOURCETABLEMODEL_H
