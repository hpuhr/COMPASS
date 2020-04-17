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

    void update();

private:
    ViewManager& view_manager_;

    QStringList table_columns_{"id", "name", "type"};

    std::map<unsigned int, ViewPoint>& view_points_;

    void updateTableColumns();
};

#endif // VIEWPOINTSTABLEMODEL_H
