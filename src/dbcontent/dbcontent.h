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

#ifndef DBOBJECT_H_
#define DBOBJECT_H_

#include "configurable.h"
#include "dboassociationcollection.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "global.h"

#include <qobject.h>

#include <memory>
#include <string>
#include <vector>

class COMPASS;
class PropertyList;

class DBContentWidget;
class Buffer;
class Job;
class DBOReadDBJob;
class InsertBufferDBJob;
class UpdateBufferDBJob;
class FinalizeDBOReadJob;
class DBContentVariableSet;
class DBOLabelDefinition;
class DBContentLabelDefinitionWidget;
class DBContentManager;

class DBContent : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void updateProgressSignal(float percent);
    void updateDoneSignal(DBContent& object);

    void labelDefinitionChangedSignal();

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

    void readJobIntermediateSlot(std::shared_ptr<Buffer> buffer);
    void readJobObsoleteSlot();
    void readJobDoneSlot();
    void finalizeReadJobDoneSlot();

    void insertDoneSlot();

    void updateProgressSlot(float percent);
    void updateDoneSlot();

public:
    static const Property meta_var_rec_num_id_;
    static const Property meta_var_datasource_id_;
    static const Property meta_var_line_id_;
    static const Property meta_var_tod_id_;
    static const Property meta_var_m3a_id_;
    static const Property meta_var_ta_id_;
    static const Property meta_var_ti_id_;
    static const Property meta_var_mc_id_;
    static const Property meta_var_track_num_id_;
    static const Property meta_var_latitude_;
    static const Property meta_var_longitude_;
    static const Property meta_var_detection_type_;

    static const Property var_radar_range_;
    static const Property var_radar_azimuth_;
    static const Property var_radar_altitude_;

    static const Property selected_var;

    DBContent(COMPASS& compass, const std::string& class_id, const std::string& instance_id,
             DBContentManager* manager);
    virtual ~DBContent();

    bool hasVariable(const std::string& name) const;
    DBContentVariable& variable(const std::string& name);
    void renameVariable(const std::string& name, const std::string& new_name);
    void deleteVariable(const std::string& name);

    const std::vector<std::unique_ptr<DBContentVariable>>& variables() const { return variables_; }

    bool hasVariableDBColumnName(const std::string& name) const;

    size_t numVariables() const { return variables_.size(); }

    const std::string& name() const { return name_; }
    void name(const std::string& name)
    {
        assert(name.size() > 0);
        name_ = name;
    }

    const std::string& info() const { return info_; }
    void info(const std::string& info) { info_ = info; }

    bool loadable() const { return is_loadable_; }

    void load(DBContentVariableSet& read_set, bool use_filters, bool use_order,
              DBContentVariable* order_variable, bool use_order_ascending,
              const std::string& limit_str = ""); // main load function
    void loadFiltered(DBContentVariableSet& read_set, std::string custom_filter_clause,
                      std::vector<DBContentVariable*> filtered_variables, bool use_order,
                      DBContentVariable* order_variable, bool use_order_ascending,
                      const std::string& limit_str = ""); // load function for custom filtering
    void quitLoading();

    void insertData(std::shared_ptr<Buffer> buffer);
    void updateData(DBContentVariable& key_var, DBContentVariableSet& list, std::shared_ptr<Buffer> buffer);

    std::map<unsigned int, std::string> loadLabelData(std::vector<unsigned int> rec_nums, int break_item_cnt);

    bool isLoading();
    bool isInserting();
    bool isPostProcessing();
    bool hasData();
    size_t count();
    size_t loadedCount();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool hasKeyVariable();
    DBContentVariable& getKeyVariable();

    std::string status();

    DBContentWidget* widget();
    void closeWidget();

    DBContentLabelDefinitionWidget* labelDefinitionWidget();

    //std::shared_ptr<Buffer> data() { return data_; }

    bool existsInDB() const;

    // association stuff, outdated
    void loadAssociationsIfRequired();  // starts loading job if required
    void loadAssociations();            // actually loads associations, should be called from job
    bool associationsLoaded() const;
    bool hasAssociations();
    void addAssociation(unsigned int rec_num, unsigned int utn, bool has_src, unsigned int src_rec_num);
    const DBOAssociationCollection& associations() { return associations_; }
    void clearAssociations();
    void saveAssociations();


    std::string dbTableName() const;

    void checkLabelDefinitions();

protected:
    COMPASS& compass_;
    DBContentManager& dbo_manager_;
    std::string name_;
    std::string info_;
    std::string db_table_name_;
    std::string ds_type_;

    bool constructor_active_ {false};

    bool is_loadable_{false};  // loadable on its own
    size_t count_{0};

    std::unique_ptr<DBOLabelDefinition> label_definition_;

    std::shared_ptr<DBOReadDBJob> read_job_{nullptr};
    std::vector<std::shared_ptr<Buffer>> read_job_data_;
    std::vector<std::shared_ptr<FinalizeDBOReadJob>> finalize_jobs_;

    bool insert_active_ {false};
    std::shared_ptr<InsertBufferDBJob> insert_job_{nullptr};
    std::shared_ptr<UpdateBufferDBJob> update_job_{nullptr};

    /// Container with all variables (variable identifier -> variable pointer)
    std::vector<std::unique_ptr<DBContentVariable>> variables_;

    std::unique_ptr<DBContentWidget> widget_;

    bool associations_changed_{false};
    bool associations_loaded_{false};
    DBOAssociationCollection associations_;

    virtual void checkSubConfigurables();

    void doDataSourcesBeforeInsert (std::shared_ptr<Buffer> buffer);

    std::string associationsTableName();

    void sortContent();

    void checkStaticVariable(const Property& property);

};

#endif /* DBOBJECT_H_ */
