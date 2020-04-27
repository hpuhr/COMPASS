#ifndef VIEWPOINTSTABLEMODEL_H
#define VIEWPOINTSTABLEMODEL_H

#include <QAbstractItemModel>

class ViewManager;
class ViewPoint;

class ViewPointsTableModel : public QAbstractItemModel
{
public:
    ViewPointsTableModel(ViewManager& view_manager);
    //virtual ~ViewPointsTableModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void update();
    unsigned int getIdOf (const QModelIndex& index);

private:
    ViewManager& view_manager_;

    QStringList table_columns_{"id", "name", "type", "status", "comment"};

    std::map<unsigned int, ViewPoint>& view_points_;

    void updateTableColumns();
};

#endif // VIEWPOINTSTABLEMODEL_H
