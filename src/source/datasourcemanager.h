#ifndef DATASOURCEMANAGER_H
#define DATASOURCEMANAGER_H

#include "configurable.h"
#include "source/configurationdatasource.h"
#include "source/dbdatasource.h"

#include <QObject>

#include <set>
#include <vector>
#include <memory>

class COMPASS;
class DataSourcesLoadWidget;

class DataSourceManager : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

public:
    const static std::vector<std::string> data_source_types_;

    DataSourceManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~DataSourceManager();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool hasConfigDataSource(unsigned int ds_id);
    dbContent::ConfigurationDataSource& configDataSource(unsigned int ds_id);

    bool hasDataSource(unsigned int ds_id);
    bool hasDataSourcesOfDBContent(const std::string dbcontent_name);
    bool canAddNewDataSourceFromConfig (unsigned int ds_id);
    void addNewDataSource (unsigned int ds_id); // be sure not to call from different thread

    dbContent::DBDataSource& dataSource(unsigned int ds_id);
    const std::vector<std::unique_ptr<dbContent::DBDataSource>>& dataSources() const;

    std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> getNetworkLines(); //ds_id -> (ip, port)

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

    void setLoadDataSources (bool loading_wanted);
    void setLoadOnlyDataSources (std::set<unsigned int> ds_ids);
    bool loadDataSourcesFiltered();
    std::set<unsigned int> getLoadDataSources ();

    DataSourcesLoadWidget* loadWidget();
    void updateWidget();

protected:
    COMPASS& compass_;

    std::vector<std::unique_ptr<dbContent::ConfigurationDataSource>> config_data_sources_;
    std::vector<std::unique_ptr<dbContent::DBDataSource>> db_data_sources_;

    std::unique_ptr<DataSourcesLoadWidget> load_widget_;

    std::map<std::string, bool> ds_type_loading_wanted_;

    virtual void checkSubConfigurables();

    void loadDBDataSources();
    void sortDBDataSources();
};

#endif // DATASOURCEMANAGER_H
