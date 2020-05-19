#ifndef VIEWPOINTSTABLEMODEL_H
#define VIEWPOINTSTABLEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include "json.hpp"

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

    unsigned int saveNewViewPoint(const nlohmann::json& data, bool update=true);
    ViewPoint& saveNewViewPoint(unsigned int id, const nlohmann::json& data, bool update=true);
    bool existsViewPoint(unsigned int id);
    ViewPoint& viewPoint(unsigned int id);
    //void removeViewPoint(unsigned int id);
    void deleteAllViewPoints ();

    std::map<unsigned int, ViewPoint>& viewPoints() { return view_points_; }
    void printViewPoints();
    //void saveViewPoints();

    void importViewPoints (const std::string& filename);
    void exportViewPoints (const std::string& filename);

    //void update();
    unsigned int getIdOf (const QModelIndex& index);

    void setStatus (const QModelIndex &row_index, const std::string& value);

    int commentColumn () { return table_columns_.indexOf("comment"); }
    int statusColumn () { return table_columns_.indexOf("status"); }

    bool updateTableColumns(); // true if changed

private:
    ViewManager& view_manager_;

    QStringList default_table_columns_ {"id", "name", "type", "status", "comment"};
    QStringList table_columns_;

    QIcon open_icon_;
    QIcon closed_icon_;
    QIcon todo_icon_;
    QIcon unknown_icon_;

    std::map<unsigned int, ViewPoint> view_points_;
};

#endif // VIEWPOINTSTABLEMODEL_H
