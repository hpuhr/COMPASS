/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "evaluationtargetdata.h"
#include "evaluationdatawidget.h"
#include "dbcontentaccessor.h"

#include <QAbstractItemModel>

#include <memory>
#include <map>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

class EvaluationCalculator;
class DBContent;
class DBContentManager;
class Buffer;

struct target_tag
{
};

namespace ResultReport
{
    class Report;
}

typedef boost::multi_index_container<
    EvaluationTargetData,
    boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,  // this index represents insertion order
        boost::multi_index::ordered_unique<boost::multi_index::tag<target_tag>,
            boost::multi_index::member<
        EvaluationTargetData, const unsigned int, &EvaluationTargetData::utn_> >
        > >
    TargetCache;

/**
 */
class EvaluationData : public QAbstractItemModel
{
    Q_OBJECT

public slots:
    void targetChangedSlot(unsigned int utn); // for one utn
    void allTargetsChangedSlot(); // for more than 1 utn

public:
    EvaluationData(EvaluationCalculator& calculator, 
                   DBContentManager& dbcont_man);

    void setBuffers(std::map<std::string, std::shared_ptr<Buffer>> buffers);
    void addReferenceData (const std::string& dbcontent_name, unsigned int line_id);
    void addTestData (const std::string& dbcontent_name, unsigned int line_id);
    void finalize ();

    bool hasTargetData (unsigned int utn);
    const EvaluationTargetData& targetData(unsigned int utn);
    unsigned int size() { return target_data_.size(); }

    typedef TargetCache::index<target_tag>::type TargetCacheIterator;
    TargetCacheIterator::const_iterator begin() { return target_data_.get<target_tag>().begin(); }
    TargetCacheIterator::const_iterator end() { return target_data_.get<target_tag>().end(); }

    void clear();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant& value, int role) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const EvaluationTargetData& getTargetOf (const QModelIndex& index);

    boost::optional<nlohmann::json> getTableData(bool rowwise = true,
                                                 const std::vector<int>& cols = std::vector<int>()) const;

    EvaluationDataWidget* widget();

    void clearInterestFactors();
    void resetModelBegin();
    void resetModelEnd();

    void setInterestFactorEnabled(const std::string& req_name, bool ok, bool update);
    void setInterestFactorEnabled(bool ok, bool update);
    bool interestFactorEnabled(const std::string& req_name) const;
    
    void updateInterestSwitches();

    const std::map<std::string, bool>& interestSwitches() const { return interest_factor_enabled_; }

    void addToReport(std::shared_ptr<ResultReport::Report> report);

    // ref
    unsigned int ref_line_id_;

    // tst
    unsigned int tst_line_id_;

    static const std::string TargetsTableName;

protected:
    void updateAllInterestFactors();

    EvaluationCalculator& calculator_;
    DBContentManager& dbcont_man_;

    QStringList table_columns_ {"Use", "UTN", "Comment", "Interest",
                               "Begin", "End", "#All", "#Ref", "#Tst", "ACIDs", "ACADs",
                                "M3/A", "MC Min", "MC Max"};

    std::shared_ptr<dbContent::DBContentAccessor> accessor_;

    TargetCache target_data_;
    bool finalized_ {false};

    EvaluationDataWidget* widget_ = nullptr;

    unsigned int unassociated_ref_cnt_ {0};
    unsigned int associated_ref_cnt_ {0};

    unsigned int unassociated_tst_cnt_ {0};
    unsigned int associated_tst_cnt_ {0};

    std::map<std::string, bool> interest_factor_enabled_; // requirement name -> bool
};
