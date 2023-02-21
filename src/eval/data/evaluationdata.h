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

#ifndef EVALUATIONDATA_H
#define EVALUATIONDATA_H

#include "evaluationtargetdata.h"
#include "evaluationdatawidget.h"

//#include <tbb/tbb.h>

#include <QAbstractItemModel>

#include <memory>
#include <map>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>


class EvaluationManager;
class DBContent;
class Buffer;

struct target_tag
{
};

typedef boost::multi_index_container<
    EvaluationTargetData,
    boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,  // this index represents insertion order
        boost::multi_index::ordered_unique<boost::multi_index::tag<target_tag>,
            boost::multi_index::member<
        EvaluationTargetData, const unsigned int, &EvaluationTargetData::utn_> >
        > >
    TargetCache;

class EvaluationData : public QAbstractItemModel
{
    Q_OBJECT

public:
    EvaluationData(EvaluationManager& eval_man);

    void addReferenceData (DBContent& object, unsigned int line_id, std::shared_ptr<Buffer> buffer);
    void addTestData (DBContent& object, unsigned int line_id, std::shared_ptr<Buffer> buffer);
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

    void setUseTargetData (unsigned int utn, bool value);
    void setUseAllTargetData (bool value);
    void clearComments ();
    void setUseByFilter ();

    void setTargetDataComment (unsigned int utn, std::string comment);

    EvaluationDataWidget* widget();

    // ref
    std::shared_ptr<Buffer> ref_buffer_;
    unsigned int ref_line_id_;

    std::string ref_timestamp_name_;
    std::string ref_latitude_name_;
    std::string ref_longitude_name_;
    std::string ref_target_address_name_;
    std::string ref_callsign_name_;

    std::string ref_modea_name_;
    std::string ref_modea_g_name_; // can be empty
    std::string ref_modea_v_name_; // can be empty

    std::string ref_modec_name_;
    std::string ref_modec_g_name_; // can be empty
    std::string ref_modec_v_name_; // can be empty
    bool has_ref_altitude_secondary_ {false};
    std::string ref_altitude_secondary_name_;

    std::string ref_ground_bit_name_; // can be empty

    std::string ref_spd_ground_speed_kts_name_; // can be empty
    std::string ref_spd_track_angle_deg_name_; // can be empty

    std::string ref_spd_x_ms_name_; // can be empty
    std::string ref_spd_y_ms_name_; // can be empty

    // tst
    std::shared_ptr<Buffer> tst_buffer_;
    unsigned int tst_line_id_;

    std::string tst_timestamp_name_;
    std::string tst_latitude_name_;
    std::string tst_longitude_name_;
    std::string tst_target_address_name_;
    std::string tst_callsign_name_;

    std::string tst_modea_name_;
    std::string tst_modea_g_name_; // can be empty
    std::string tst_modea_v_name_; // can be empty

    std::string tst_modec_name_;
    std::string tst_modec_g_name_; // can be empty
    std::string tst_modec_v_name_; // can be empty

    std::string tst_ground_bit_name_; // can be empty

    std::string tst_track_num_name_; // can be empty

    std::string tst_spd_ground_speed_kts_name_; // can be empty
    std::string tst_spd_track_angle_deg_name_; // can be empty

    std::string tst_spd_x_ms_name_; // can be empty
    std::string tst_spd_y_ms_name_; // can be empty

    std::string tst_multiple_srcs_name_; // can be empty TODO
    std::string tst_track_lu_ds_id_name_; // can be empty TODO


protected:
    EvaluationManager& eval_man_;

    QStringList table_columns_ {"Use", "UTN", "Comment", "Begin", "End", "#All", "#Ref", "#Tst", "Callsign", "TA",
                                "M3/A", "MC Min", "MC Max"};

    TargetCache target_data_;
    bool finalized_ {false};

    std::unique_ptr<EvaluationDataWidget> widget_;

    unsigned int unassociated_ref_cnt_ {0};
    unsigned int associated_ref_cnt_ {0};

    unsigned int unassociated_tst_cnt_ {0};
    unsigned int associated_tst_cnt_ {0};
};

#endif // EVALUATIONDATA_H
