#ifndef VIEWPOINTSTABLEMODEL_H
#define VIEWPOINTSTABLEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include "json.hpp"

#include "viewpoint.h"

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

class ViewManager;

struct vp_tag
{
};

typedef boost::multi_index_container<
    ViewPoint,
    boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,  // this index represents insertion order
        boost::multi_index::hashed_unique<boost::multi_index::tag<vp_tag>,
            boost::multi_index::member<
        ViewPoint, const unsigned int, &ViewPoint::id_> >
        > >
    ViewPointCache;

class ViewPointsTableModel : public QAbstractItemModel
{
    Q_OBJECT

signals:
    void typesChangedSignal(QStringList types);
    void statusesChangedSignal(QStringList types);

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


    bool hasViewPoint (unsigned int id);
    unsigned int saveNewViewPoint(const nlohmann::json& data, bool update=true);
    const ViewPoint& saveNewViewPoint(unsigned int id, const nlohmann::json& data, bool update=true);
    //bool existsViewPoint(unsigned int id);
    const ViewPoint& viewPoint(unsigned int id);
    //void removeViewPoint(unsigned int id);
    void deleteAllViewPoints ();

    //std::map<unsigned int, ViewPoint>& viewPoints() { return view_points_; }
    void printViewPoints();
    //void saveViewPoints();

    void importViewPoints (const std::string& filename);
    void exportViewPoints (const std::string& filename);

    //void update();
    unsigned int getIdOf (const QModelIndex& index);

    void setStatus (const QModelIndex &row_index, const std::string& value);

    int typeColumn () { return table_columns_.indexOf("type"); }
    int statusColumn () { return table_columns_.indexOf("status"); }
    int commentColumn () { return table_columns_.indexOf("comment"); }
    //int columnIndex (QString name);

    bool updateTableColumns(); // true if changed
    void updateTypes(); // emits signal if changed
    void updateStatuses(); // emits signal if changed

    QStringList types() const;
    QStringList statuses() const;
    QStringList tableColumns() const;

    QStringList defaultTableColumns() const;

    const ViewPointCache& viewPoints() const;

private:
    ViewManager& view_manager_;

    QStringList default_table_columns_ {"id", "name", "type", "status", "comment"};
    QStringList table_columns_;

    QStringList types_;
    QStringList statuses_;

    QIcon open_icon_;
    QIcon closed_icon_;
    QIcon todo_icon_;
    QIcon unknown_icon_;

    //std::map<unsigned int, ViewPoint> view_points_;
    ViewPointCache view_points_;
    unsigned int max_id_ {0};
};

#endif // VIEWPOINTSTABLEMODEL_H
