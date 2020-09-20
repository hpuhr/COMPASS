#ifndef EVALUATIONDATA_H
#define EVALUATIONDATA_H

#include "evaluationtargetdata.h"
#include "evaluationdatawidget.h"

#include <QAbstractItemModel>

#include <memory>
#include <map>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

class EvaluationManager;
class DBObject;
class Buffer;

struct target_tag
{
};

typedef boost::multi_index_container<
    EvaluationTargetData,
    boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,  // this index represents insertion order
        boost::multi_index::hashed_unique<boost::multi_index::tag<target_tag>,
            boost::multi_index::member<
        EvaluationTargetData, const unsigned int, &EvaluationTargetData::utn_> >
        > >
    TargetCache;

class EvaluationData : public QAbstractItemModel
{
    Q_OBJECT

public:
    EvaluationData(EvaluationManager& eval_man);

    void addReferenceData (DBObject& object, std::shared_ptr<Buffer> buffer);
    void addTestData (DBObject& object, std::shared_ptr<Buffer> buffer);
    void finalize ();

    bool hasTargetData (unsigned int utn);
    const EvaluationTargetData& targetData(unsigned int utn);

    void clear();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    EvaluationDataWidget* widget();

protected:
    EvaluationManager& eval_man_;

    QStringList table_columns_ {"UTN", "Begin", "End", "Updates"};

    TargetCache target_data_;
    bool finalized_ {false};

    std::unique_ptr<EvaluationDataWidget> widget_;

    unsigned int unassociated_ref_cnt_ {0};
    unsigned int associated_ref_cnt_ {0};

    unsigned int unassociated_tst_cnt_ {0};
    unsigned int associated_tst_cnt_ {0};
};

#endif // EVALUATIONDATA_H
