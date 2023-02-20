#ifndef DBCONTENT_TARGETMODEL_H
#define DBCONTENT_TARGETMODEL_H

#include "target.h"

#include <QAbstractItemModel>

#include <memory>
#include <map>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>


namespace dbContent {

struct target_tag
{
};

typedef boost::multi_index_container<
    Target,
    boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,  // this index represents insertion order
        boost::multi_index::ordered_unique<boost::multi_index::tag<target_tag>,
            boost::multi_index::member<
        Target, const unsigned int, &Target::utn_> >
        > >
    TargetCache;

class TargetModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TargetModel();
    virtual ~TargetModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant& value, int role) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const dbContent::Target& getTargetOf (const QModelIndex& index);

    void setUseTargetData (unsigned int utn, bool value);
    void setUseAllTargetData (bool value);
    void clearComments ();
    void setUseByFilter ();

    void setTargetDataComment (unsigned int utn, std::string comment);

    bool hasTargetsInfo();
    void clearTargetsInfo();
    bool existsTarget(unsigned int utn);
    void createNewTarget(unsigned int utn);
    dbContent::Target& target(unsigned int utn);

    void loadFromDB();
    void saveToDB();
    void saveToDB(unsigned int utn);

    void clear();

protected:
    //EvaluationManager& eval_man_;

    QStringList table_columns_ {"Use", "UTN", "Comment", "#Updates", "Begin", "End", "ACIDs", "ACADs",
                                "M3/A", "MC Min", "MC Max", "MOPS"};

    TargetCache target_data_;
};

}

#endif // DBCONTENT_TARGETMODEL_H
