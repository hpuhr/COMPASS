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
//#include "dbodatasource.h"
//#include "dbodatasourcedefinition.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "global.h"

#include <qobject.h>

#include <memory>
#include <string>
#include <vector>

class COMPASS;
class PropertyList;

class DBObjectWidget;
class DBObjectInfoWidget;
class Buffer;
class Job;
class DBOReadDBJob;
class InsertBufferDBJob;
class UpdateBufferDBJob;
class FinalizeDBOReadJob;
class DBOVariableSet;
class DBOLabelDefinition;
class DBOLabelDefinitionWidget;
class DBObjectManager;

class DBObject : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void newDataSignal(DBObject& object);
    void loadingDoneSignal(DBObject& object);

    void insertProgressSignal(float percent);
    void insertDoneSignal(DBObject& object);

    void updateProgressSignal(float percent);
    void updateDoneSignal(DBObject& object);

    void labelDefinitionChangedSignal();

  public slots:
    void readJobIntermediateSlot(std::shared_ptr<Buffer> buffer);
    void readJobObsoleteSlot();
    void readJobDoneSlot();
    void finalizeReadJobDoneSlot();

    void insertProgressSlot(float percent);
    void insertDoneSlot();

    void updateProgressSlot(float percent);
    void updateDoneSlot();

  public:
    static const std::string var_name_datasource_id_;

    DBObject(COMPASS& compass, const std::string& class_id, const std::string& instance_id,
             DBObjectManager* manager);
    virtual ~DBObject();

    bool hasVariable(const std::string& name) const;
    DBOVariable& variable(const std::string& name);
    void renameVariable(const std::string& name, const std::string& new_name);
    void deleteVariable(const std::string& name);

    const std::vector<std::unique_ptr<DBOVariable>>& variables() const { return variables_; }

    bool hasVariableDBColumnName(const std::string& name) const;

//    using DBOVariableIterator = typename std::vector<std::unique_ptr<DBOVariable>>::iterator;
//    DBOVariableIterator begin() { return variables_.begin(); }
//    DBOVariableIterator end() { return variables_.end(); }

//    using const_DBOVariableIterator = typename std::vector<std::unique_ptr<DBOVariable>>::const_iterator;
//    const_DBOVariableIterator cbegin() const { return variables_.cbegin(); }
//    const_DBOVariableIterator cend() const { return variables_.cend(); }

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

    void loadingWanted(bool wanted);
    bool loadingWanted() { return loading_wanted_; }

    void load(DBOVariableSet& read_set, bool use_filters, bool use_order,
              DBOVariable* order_variable, bool use_order_ascending,
              const std::string& limit_str = "");
    void load(DBOVariableSet& read_set, std::string custom_filter_clause,
              std::vector<DBOVariable*> filtered_variables, bool use_order,
              DBOVariable* order_variable, bool use_order_ascending,
              const std::string& limit_str = "");
    void quitLoading();
    void clearData();

    void insertData(DBOVariableSet& list, std::shared_ptr<Buffer> buffer, bool emit_change = true);
    void updateData(DBOVariable& key_var, DBOVariableSet& list, std::shared_ptr<Buffer> buffer);

    std::map<int, std::string> loadLabelData(std::vector<int> rec_nums, int break_item_cnt);

    bool isLoading();
    bool isPostProcessing();
    bool hasData();
    size_t count();
    size_t loadedCount();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool hasKeyVariable();
    DBOVariable& getKeyVariable();

    std::string status();

    DBObjectWidget* widget();
    void closeWidget();

    DBObjectInfoWidget* infoWidget();
    DBOLabelDefinitionWidget* labelDefinitionWidget();

    std::shared_ptr<Buffer> data() { return data_; }

    bool existsInDB() const;

    // association stuff
    void loadAssociationsIfRequired();  // starts loading job if required
    void loadAssociations();            // actually loads associations, should be called from job
    bool associationsLoaded() const;
    bool hasAssociations();
    void addAssociation(unsigned int rec_num, unsigned int utn, bool has_src, unsigned int src_rec_num);
    const DBOAssociationCollection& associations() { return associations_; }
    void clearAssociations();
    void saveAssociations();

    void updateToDatabaseContent();

    std::string dbTableName() const;

protected:
    COMPASS& compass_;
    DBObjectManager& manager_;
    std::string name_;
    std::string info_;
    std::string db_table_name_;

    bool constructor_active_ {false};

    bool is_loadable_{false};  // loadable on its own
    bool loading_wanted_{false};
    size_t count_{0};

    std::unique_ptr<DBOLabelDefinition> label_definition_;

    std::shared_ptr<DBOReadDBJob> read_job_{nullptr};
    std::vector<std::shared_ptr<Buffer>> read_job_data_;
    std::vector<std::shared_ptr<FinalizeDBOReadJob>> finalize_jobs_;

    std::shared_ptr<InsertBufferDBJob> insert_job_{nullptr};
    std::shared_ptr<UpdateBufferDBJob> update_job_{nullptr};

    std::shared_ptr<Buffer> data_;

    /// Container with all variables (variable identifier -> variable pointer)
    std::vector<std::unique_ptr<DBOVariable>> variables_;

    std::unique_ptr<DBObjectWidget> widget_;
    std::unique_ptr<DBObjectInfoWidget> info_widget_;

    bool associations_changed_{false};
    bool associations_loaded_{false};
    DBOAssociationCollection associations_;

    virtual void checkSubConfigurables();

    void doDataSourcesBeforeInsert (std::shared_ptr<Buffer> buffer);

    std::string associationsTableName();

    void sortContent();

};

#endif /* DBOBJECT_H_ */
