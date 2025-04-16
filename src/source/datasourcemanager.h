
#pragma once

#include "configurable.h"
#include "source/configurationdatasource.h"
#include "source/dbdatasource.h"
#include "json.hpp"

#include <QObject>

#include <set>
#include <vector>
#include <memory>

class COMPASS;
class DataSourcesWidget;
class DataSourcesConfigurationDialog;

class DataSourceManager : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void dataSourcesChangedSignal();

  public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

    void configurationDialogDoneSlot();

  public:
    struct Config
    {
        Config();

        bool load_widget_show_counts_ {false};
        bool load_widget_show_lines_ {false};

        unsigned int ds_font_size_ {10};

        double primary_azimuth_stddev_;   // degrees
        double primary_range_stddev_;     // meters
        double secondary_azimuth_stddev_; // degrees
        double secondary_range_stddev_;   // meters
        double mode_s_azimuth_stddev_;    // degrees
        double mode_s_range_stddev_;      // meters

    };

    const static std::vector<std::string> data_source_types_;

    DataSourceManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~DataSourceManager();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    const std::vector<unsigned int>& getAllDsIDs(); // both config and db

    bool hasConfigDataSource(unsigned int ds_id);
    void createConfigDataSource(unsigned int ds_id);
    void deleteConfigDataSource(unsigned int ds_id);
    dbContent::ConfigurationDataSource& configDataSource(unsigned int ds_id);
    const std::vector<std::unique_ptr<dbContent::ConfigurationDataSource>>& configDataSources() const;

    bool hasDBDataSource(unsigned int ds_id);
    bool hasDBDataSource(const std::string& ds_name);
    unsigned int getDBDataSourceDSID(const std::string& ds_name);
    bool hasDataSourcesOfDBContent(const std::string dbcontent_name);
    bool canAddNewDataSourceFromConfig (unsigned int ds_id);
    void addNewDataSource (unsigned int ds_id); // be sure not to call from different thread

    dbContent::DBDataSource& dbDataSource(unsigned int ds_id);
    const std::vector<std::unique_ptr<dbContent::DBDataSource>>& dbDataSources() const;

    std::set<unsigned int> groundOnlyDBDataSources() const;

    void createNetworkDBDataSources();
    std::map<unsigned int, std::map<std::string, std::shared_ptr<DataSourceLineInfo>>> getNetworkLines() const;
    //ds_id -> line str ->(ip, port)

    void saveDBDataSources();

    bool dsTypeFiltered();
    std::set<std::string> wantedDSTypes();
    void dsTypeLoadingWanted (const std::string& ds_type, bool wanted);
    bool dsTypeLoadingWanted (const std::string& ds_type);
    void setLoadDSTypes (bool loading_wanted);
    void setLoadOnlyDSTypes (std::set<std::string> ds_types);

    bool loadingWanted (const std::string& dbcontent_name);
    bool hasDSFilter (const std::string& dbcontent_name);
    std::vector<unsigned int> unfilteredDS (const std::string& dbcontent_name); // DS IDs
    bool lineSpecificLoadingRequired(const std::string& dbcontent_name);

    void setLoadDataSources (bool loading_wanted);
    void setLoadAllDataSourceLines ();
    void setLoadOnlyDataSources (std::map<unsigned int, std::set<unsigned int>> ds_ids); // ds_id + line strs
    bool loadDataSourcesFiltered();
    std::map<unsigned int, std::set<unsigned int>> getLoadDataSources (); // ds_id -> wanted lines

    void resetToStartupConfiguration();

    DataSourcesWidget* loadWidget();
    void updateWidget();

    DataSourcesConfigurationDialog* configurationDialog();

    void importDataSources(const std::string& filename); // import data sources from json
    void importDataSourcesJSONDeprecated(const nlohmann::json& j);
    void importDataSourcesJSON(const nlohmann::json& j);

    void deleteAllConfigDataSources();
    void exportDataSources(const std::string& filename);

    nlohmann::json getDataSourcesAsJSON();
    nlohmann::json getDBDataSourcesAsJSON();
    nlohmann::json getSortedConfigDataSourcesAsJSON();
    nlohmann::json getSortedDBDataSourcesAsJSON();

    void setLoadedCounts(std::map<unsigned int, std::map<std::string,
                                                         std::map<unsigned int, unsigned int>>> loaded_counts); // ds id->dbcont->line->cnt
    void clearInsertedCounts(const std::string& dbcontent_name); // after delete all dbcontent

//    bool loadWidgetShowCounts() const;
//    void loadWidgetShowCounts(bool value);

//    bool loadWidgetShowLines() const;
//    void loadWidgetShowLines(bool value);

//    unsigned int dsFontSize() const;

            // selection stuff

    void selectAllDSTypes();
    void deselectAllDSTypes();

    void selectAllDataSources();
    void deselectAllDataSources();

    void selectDSTypeSpecificDataSources (const std::string& ds_type);
    void deselectDSTypeSpecificDataSources (const std::string& ds_type);

    void deselectAllLines();
    void selectSpecificLineSlot(unsigned int line_id);

    DataSourceManager::Config& config();

  protected:
    COMPASS& compass_;

    Config config_;

    std::vector<std::unique_ptr<dbContent::ConfigurationDataSource>> config_data_sources_;
    std::vector<std::unique_ptr<dbContent::DBDataSource>> db_data_sources_;
    std::vector<unsigned int> ds_ids_all_; // both from config and db, vector to have order

    std::unique_ptr<DataSourcesWidget> load_widget_;

    std::unique_ptr<DataSourcesConfigurationDialog> config_dialog_;

    std::map<std::string, bool> ds_type_loading_wanted_; // if not in there, wanted

    virtual void checkSubConfigurables();

    void loadDBDataSources();
    void sortDBDataSources();

    void updateDSIdsAll();
    void createConfigDataSourcesFromDB();
};
