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

#ifndef DBCONTENT_DBCONTENTMANAGER_H_
#define DBCONTENT_DBCONTENTMANAGER_H_

#include "configurable.h"
#include "global.h"
#include "singleton.h"
#include "buffer.h"
#include "targetmodel.h"
#include "dbcontent/dbcontentcache.h"
#include "viewabledataconfig.h"

#include <boost/optional.hpp>

#include <QObject>

#include <vector>
#include <memory>

class COMPASS;
class DBContent;
class DBContentManagerWidget;
class DBSchemaManager;
class DBContentDeleteDBJob;

namespace dbContent {

class MetaVariableConfigurationDialog;
class Variable;
class MetaVariable;
class VariableSet;
class Target;
class TargetListWidget;
class LabelGenerator;
class VariableSet;

}

class DBContentManager : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

    void deleteJobDoneSlot();

    void metaDialogOKSlot();

signals:
    void dbObjectsChangedSignal();
    void associationStatusChangedSignal();

    void loadingStartedSignal(); // emitted when load has been started
    // all data contained, also new one. requires_reset true indicates that all shown info should be re-created,
    // e.g. when data in the beginning was removed, or order of previously emitted data was changed, etc.
    void loadedDataSignal (const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSignal(); // emitted when all dbos have finished loading
    void insertDoneSignal(); // emitted when all dbos have finished loading

    // if useInEval or comment changed signals, to be sent from model
    void targetChangedSignal(unsigned int utn); // for one utn
    void allTargetsChangedSignal(); // for more than 1 utn

public:
    DBContentManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~DBContentManager();

    virtual void generateSubConfigurable(const std::string& class_id, const std::string& instance_id) override;

    dbContent::LabelGenerator& labelGenerator();

    bool existsDBContent(const std::string& dbcontent_name);
    DBContent& dbContent(const std::string& dbcontent_name);
    void deleteDBContent(const std::string& dbcontent_name);
    bool hasData();

    using DBContentIterator = typename std::map<std::string, DBContent*>::iterator;
    DBContentIterator begin() { return dbcontent_.begin(); }
    DBContentIterator end() { return dbcontent_.end(); }
    size_t size() { return dbcontent_.size(); }

    unsigned int getMaxDBContentID();
    bool existsDBContentWithId (unsigned int id);
    const std::string& dbContentWithId (unsigned int id);

    bool existsMetaVariable(const std::string& var_name);
    dbContent::MetaVariable& metaVariable(const std::string& var_name);
    void renameMetaVariable(const std::string& old_var_name, const std::string& new_var_name);
    void deleteMetaVariable(const std::string& var_name);
    const std::map<std::string, std::unique_ptr<dbContent::MetaVariable>>& metaVariables() { return meta_variables_; }

    bool usedInMetaVariable(const dbContent::Variable& variable);
    dbContent::MetaVariableConfigurationDialog* metaVariableConfigdialog();

    void load(const std::string& custom_filter_clause="");
    void addLoadedData(std::map<std::string, std::shared_ptr<Buffer>> data);
    std::map<std::string, std::shared_ptr<Buffer>> loadedData();
    void loadingDone(DBContent& object); // to be called by dbo when it's loading is finished
    bool loadInProgress() const;
    void clearData();

    void insertData(std::map<std::string, std::shared_ptr<Buffer>> data);
    void insertDone(DBContent& object); // to be called by dbo when it's insert is finished
    bool insertInProgress() const;

    void deleteDBContentData(boost::posix_time::ptime before_timestamp);

    DBContentManagerWidget* widget();

    void quitLoading();

    bool hasAssociations() const;
    void setAssociationsIdentifier(const std::string& assoc_id);
    std::string associationsID() const;

    bool hasMaxRecordNumber() const { return has_max_rec_num_; }
    unsigned long maxRecordNumber() const;
    void maxRecordNumber(unsigned long value);

    bool hasMaxRefTrajTrackNum() const { return has_max_reftraj_track_num_; }
    unsigned int maxRefTrajTrackNum() const;
    void maxRefTrajTrackNum(unsigned int value);

    bool hasMinMaxInfo() const;
    bool hasMinMaxTimestamp() const;
    void setMinMaxTimestamp(boost::posix_time::ptime min, boost::posix_time::ptime max);
    std::pair<boost::posix_time::ptime, boost::posix_time::ptime> minMaxTimestamp() const;

    bool hasMinMaxPosition() const;
    void setMinMaxLatitude(double min, double max);
    std::pair<double, double> minMaxLatitude() const;
    void setMinMaxLongitude(double min, double max);
    std::pair<double, double> minMaxLongitude() const;

    const std::map<std::string, std::shared_ptr<Buffer>>& data() const;

    bool canGetVariable (const std::string& dbcont_name, const Property& property);
    dbContent::Variable& getVariable (const std::string& dbcont_name, const Property& property);

    bool metaCanGetVariable (const std::string& dbcont_name, const Property& meta_property);
    dbContent::Variable& metaGetVariable (const std::string& dbcont_name, const Property& meta_property);

    bool hasTargetsInfo() const;
    void clearTargetsInfo();
    bool existsTarget(unsigned int utn);
    void createNewTarget(unsigned int utn);
    dbContent::Target& target(unsigned int utn);
    void removeDBContentFromTargets(const std::string& dbcont_name);
    void loadTargets();
    void saveTargets();

    nlohmann::json targetsInfoAsJSON() const;
    nlohmann::json targetInfoAsJSON(unsigned int utn) const;
    nlohmann::json utnsAsJSON() const;

    unsigned int maxLiveDataAgeCache() const;

    void resetToStartupConfiguration(); // only resets label generator

    dbContent::TargetListWidget* targetListWidget();
    void resizeTargetListWidget();

    bool utnUseEval (unsigned int utn);
    void utnUseEval (unsigned int utn, bool value);

    std::string utnComment (unsigned int utn);
    void utnComment (unsigned int utn, std::string value);

    void autoFilterUTNS();
    void showUTN (unsigned int utn);

protected:
    COMPASS& compass_;

    std::unique_ptr<dbContent::LabelGenerator> label_generator_;
    std::unique_ptr<dbContent::TargetModel> target_model_;
    std::unique_ptr<dbContent::TargetListWidget> target_list_widget_;

    bool has_associations_{false};
    std::string associations_id_;

    bool has_max_rec_num_ {false};
    unsigned long max_rec_num_ {0};

    bool has_max_reftraj_track_num_ {false};
    unsigned int max_reftraj_track_num_ {0};

    unsigned int max_live_data_age_cache_ {5};
    unsigned int max_live_data_age_db_ {60};

    boost::optional<boost::posix_time::ptime> timestamp_min_;
    boost::optional<boost::posix_time::ptime> timestamp_max_;
    boost::optional<double> latitude_min_;
    boost::optional<double> latitude_max_;
    boost::optional<double> longitude_min_;
    boost::optional<double> longitude_max_;

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::map<std::string, std::shared_ptr<Buffer>> insert_data_;

    bool load_in_progress_{false};
    bool insert_in_progress_{false};

    /// Container with all DBContent (DBContent name -> dbcont pointer)
    std::map<std::string, DBContent*> dbcontent_;
    std::map<unsigned int, DBContent*> dbcontent_ids_;
    std::map<std::string, std::unique_ptr<dbContent::MetaVariable>> meta_variables_;

    //std::map<unsigned int, std::shared_ptr<dbContent::Target>> targets_;

    std::unique_ptr<DBContentManagerWidget> widget_;

    std::unique_ptr<dbContent::MetaVariableConfigurationDialog> meta_cfg_dialog_;

    std::shared_ptr<DBContentDeleteDBJob> delete_job_{nullptr};

    std::unique_ptr<ViewableDataConfig> viewable_data_cfg_;

    virtual void checkSubConfigurables() override;
    void finishLoading();
    void finishInserting();

    void addInsertedDataToChache();
    void filterDataSources();
    void cutCachedData();

    void updateNumLoadedCounts(); // from data_

    void loadMaxRecordNumber();
    void loadMaxRefTrajTrackNum();

    void addStandardVariables(std::string dbcont_name, dbContent::VariableSet& read_set);

    void setViewableDataConfig (const nlohmann::json::object_t& data);
};

#endif /* DBCONTENT_DBCONTENTMANAGER_H_ */
