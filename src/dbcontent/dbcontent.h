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

#ifndef DBCONTENT_DBCONTENT_H_
#define DBCONTENT_DBCONTENT_H_

#include "configurable.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
//#include "global.h"

#include <QObject>

#include <memory>
#include <string>
#include <vector>

class COMPASS;
class PropertyList;

class DBContentWidget;
class Buffer;
class Job;
class DBContentReadDBJob;
class InsertBufferDBJob;
class UpdateBufferDBJob;
class DBContentManager;
class DBContentDeleteDBJob;

namespace dbContent
{
class VariableSet;
}

class DBContent : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void updateProgressSignal(float percent);
    void updateDoneSignal(DBContent& dbcontent);

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

    void readJobIntermediateSlot(std::shared_ptr<Buffer> buffer);
    void readJobObsoleteSlot();
    void readJobDoneSlot();

    void insertDoneSlot();

    void updateProgressSlot(float percent);
    void updateDoneSlot();

    void deleteJobDoneSlot();

public:
    static const Property meta_var_rec_num_;
    static const Property meta_var_ds_id_;
    static const Property meta_var_sac_id_;
    static const Property meta_var_sic_id_;
    static const Property meta_var_line_id_;
    static const Property meta_var_time_of_day_;
    static const Property meta_var_timestamp_; // boost::posix_time::ptime
    static const Property meta_var_m3a_;
    static const Property meta_var_m3a_g_;
    static const Property meta_var_m3a_v_;
    static const Property meta_var_m3a_smoothed_;
    static const Property meta_var_acad_;
    static const Property meta_var_acid_;
    static const Property meta_var_mc_;
    static const Property meta_var_mc_g_;
    static const Property meta_var_mc_v_;
    static const Property meta_var_ground_bit_;

    static const Property meta_var_track_num_;
    static const Property meta_var_track_begin_;
    static const Property meta_var_track_confirmed_;
    static const Property meta_var_track_coasting_;
    static const Property meta_var_track_end_;

    static const Property meta_var_latitude_;
    static const Property meta_var_longitude_;
    static const Property meta_var_detection_type_;

    static const Property meta_var_artas_hash_;
    static const Property meta_var_utn_;

    static const Property meta_var_vx_;
    static const Property meta_var_vy_;
    static const Property meta_var_ground_speed_; // kts
    static const Property meta_var_track_angle_; // deg
    static const Property meta_var_horizontal_man_;

    static const Property meta_var_ax_;
    static const Property meta_var_ay_;

    static const Property meta_var_mom_long_acc_;
    static const Property meta_var_mom_trans_acc_;
    static const Property meta_var_mom_vert_rate_;

    static const Property meta_var_x_stddev_;
    static const Property meta_var_y_stddev_;
    static const Property meta_var_xy_cov_;

    static const Property meta_var_latitude_stddev_;
    static const Property meta_var_longitude_stddev_;
    static const Property meta_var_latlon_cov_;

    static const Property meta_var_climb_descent_;
    static const Property meta_var_rocd_;
    static const Property meta_var_spi_;

    static const Property var_radar_range_;
    static const Property var_radar_azimuth_;
    static const Property var_radar_altitude_;

    static const Property var_cat021_mops_version_;
    static const Property var_cat021_nacp_;
    static const Property var_cat021_nucp_nic_;
    static const Property var_cat021_nucv_nacv_;
    static const Property var_cat021_sil_;

    static const Property var_cat062_tris_;
    static const Property var_cat062_tri_recnums_;
    static const Property var_cat062_track_begin_;
    static const Property var_cat062_coasting_;
    static const Property var_cat062_track_end_;
    static const Property var_cat062_mono_sensor_;
    static const Property var_cat062_type_lm_;
    static const Property var_cat062_baro_alt_;
    static const Property var_cat062_fl_measured_; // trusted, not valid

    //Rate of Climb/Descent float feet / min
    //Ax Ay float m/s2
    // trans long vert "MOM Longitudinal Acc" "MOM Transversal Acc" "MOM Vertical Rate" uchar

    static const Property var_cat062_wtc_;
    static const Property var_cat062_callsign_fpl_;

    static const Property var_cat062_vx_stddev_;
    static const Property var_cat062_vy_stddev_;

    static const Property selected_var;

    DBContent(COMPASS& compass, const std::string& class_id, const std::string& instance_id,
             DBContentManager* manager);
    virtual ~DBContent();

    bool hasVariable(const std::string& name) const;
    dbContent::Variable& variable(const std::string& name) const;
    void renameVariable(const std::string& name, const std::string& new_name);
    void deleteVariable(const std::string& name);

    const std::map<std::string, std::unique_ptr<dbContent::Variable>>& variables() const { return variables_; }

    bool hasVariableDBColumnName(const std::string& col_name) const;

    size_t numVariables() const { return variables_.size(); }

    const std::string& name() const { return name_; }
    void name(const std::string& name)
    {
        assert(name.size() > 0);
        name_ = name;
    }

    unsigned int id();

    const std::string& info() const { return info_; }
    void info(const std::string& info) { info_ = info; }

    bool loadable() const { return is_loadable_; }

    void load(dbContent::VariableSet& read_set, bool use_datasrc_filters, bool use_filters,
              const std::string& custom_filter_clause=""); // main load function
    void loadFiltered(dbContent::VariableSet& read_set, std::string custom_filter_clause);
    // load function for custom filtering
    void quitLoading();

    void insertData(std::shared_ptr<Buffer> buffer);
    void updateData(dbContent::Variable& key_var, std::shared_ptr<Buffer> buffer);

    // counts and targets have to be adjusted outside
    void deleteDBContentData();
    void deleteDBContentData(unsigned int sac, unsigned int sic);
    void deleteDBContentData(unsigned int sac, unsigned int sic, unsigned int line_id);

    //std::map<unsigned int, std::string> loadLabelData(std::vector<unsigned int> rec_nums, int break_item_cnt);

    bool isLoading();
    bool isInserting();
    bool isDeleting();
    //bool isPostProcessing();
    bool hasData();
    size_t count();
    size_t loadedCount();

    virtual void generateSubConfigurable(const std::string& class_id, const std::string& instance_id);

    bool hasKeyVariable();
    dbContent::Variable& getKeyVariable();

    std::string status();

    DBContentWidget* widget();
    void closeWidget();

    bool existsInDB() const;

    std::string dbTableName() const;

protected:
    COMPASS& compass_;
    DBContentManager& dbcont_manager_;
    std::string name_;
    unsigned int id_ {0};
    std::string info_;
    std::string db_table_name_;
    std::string ds_type_;

    //bool constructor_active_ {false};

    bool is_loadable_{false};  // loadable on its own
    size_t count_{0};

    std::shared_ptr<DBContentReadDBJob> read_job_{nullptr};

    bool insert_active_ {false};
    std::shared_ptr<InsertBufferDBJob> insert_job_{nullptr};
    std::shared_ptr<UpdateBufferDBJob> update_job_{nullptr};
    std::shared_ptr<DBContentDeleteDBJob> delete_job_{nullptr};

    /// Container with all variables (variable identifier -> variable pointer)
    std::map<std::string, std::unique_ptr<dbContent::Variable>> variables_;

    std::unique_ptr<DBContentWidget> widget_;

    virtual void checkSubConfigurables();

    void doDataSourcesBeforeInsert (std::shared_ptr<Buffer> buffer);

    //std::string associationsTableName();

    //void sortContent();

    void checkStaticVariable(const Property& property);

};

#endif /* DBCONTENT_DBCONTENT_H_ */
