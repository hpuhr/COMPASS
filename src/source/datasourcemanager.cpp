#include "datasourcemanager.h"
#include "datasourcesloadwidget.h"
#include "datasourcesconfigurationdialog.h"
#include "compass.h"
#include "dbinterface.h"
#include "stringconv.h"
#include "number.h"
#include "json.hpp"

using namespace std;
using namespace Utils;
using namespace dbContent;
using namespace nlohmann;

const std::vector<std::string> DataSourceManager::data_source_types_ {"Radar", "MLAT", "ADSB", "Tracker", "RefTraj"};

DataSourceManager::DataSourceManager(const std::string& class_id, const std::string& instance_id,
                                     COMPASS* compass)
        : Configurable(class_id, instance_id, compass, "data_sources.json"), compass_(*compass)
{
    createSubConfigurables();

    updateDSIdsAll();
}

DataSourceManager::~DataSourceManager()
{
    load_widget_ = nullptr;
}

void DataSourceManager::generateSubConfigurable(const std::string& class_id,
                                              const std::string& instance_id)
{
    if (class_id.compare("ConfigurationDataSource") == 0)
        {
            unique_ptr<dbContent::ConfigurationDataSource> ds {
                new dbContent::ConfigurationDataSource(class_id, instance_id, *this)};
            loginf << "DataSourceManager: generateSubConfigurable: adding config ds "
                   << ds->name() << " sac/sic " <<  ds->sac() << "/" << ds->sic();

            assert (!hasConfigDataSource(Number::dsIdFrom(ds->sac(), ds->sic())));
            config_data_sources_.emplace_back(move(ds));
        }
        else
            throw std::runtime_error("DataSourceManager: generateSubConfigurable: unknown class_id " +
                                     class_id);
}

std::vector<unsigned int> DataSourceManager::getAllDsIDs()
{
    return ds_ids_all_;
}

DataSourcesLoadWidget* DataSourceManager::loadWidget()
{
    if (!load_widget_)
    {
        load_widget_.reset(new DataSourcesLoadWidget(*this));
    }

    assert(load_widget_);
    return load_widget_.get();
}

void DataSourceManager::updateWidget()
{
    if (load_widget_)
        load_widget_->update();
}

DataSourcesConfigurationDialog* DataSourceManager::configurationDialog()
{
    if (!config_dialog_)
    {
        config_dialog_.reset(new DataSourcesConfigurationDialog(*this));

        connect (config_dialog_.get(), &DataSourcesConfigurationDialog::doneSignal,
                 this, &DataSourceManager::configurationDialogDoneSlot);
    }

    return config_dialog_.get();
}


bool DataSourceManager::loadingWanted (const std::string& dbcontent_name)
{
    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted() && ds_it->hasNumInserted(dbcontent_name))
            return true;
    }

    return false;
}

bool DataSourceManager::hasDSFilter (const std::string& dbcontent_name)
{
    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && !ds_it->loadingWanted() && ds_it->hasNumInserted(dbcontent_name))
            return true;
    }

    return false;
}

std::vector<unsigned int> DataSourceManager::unfilteredDS (const std::string& dbcontent_name)
{
    assert (hasDSFilter(dbcontent_name));

    std::vector<unsigned int> ds_ids;

    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted() && ds_it->hasNumInserted(dbcontent_name))
            ds_ids.push_back(ds_it->id());
    }

    assert (ds_ids.size());

    return ds_ids;
}

void DataSourceManager::setLoadDataSources (bool loading_wanted)
{
    loginf << "DataSourceManager: setLoadDataSources: wanted " << loading_wanted;

    for (auto& ds_it : db_data_sources_)
        ds_it->loadingWanted(loading_wanted);

    if (load_widget_)
        load_widget_->update();
}

void DataSourceManager::setLoadOnlyDataSources (std::set<unsigned int> ds_ids)
{
    loginf << "DataSourceManager: setLoadOnlyDataSources";

    // deactivate all loading
    setLoadDataSources(false);

    for (auto ds_id_it : ds_ids)
    {
        assert (hasDBDataSource(ds_id_it));
        dbDataSource(ds_id_it).loadingWanted(true);
    }

    if (load_widget_)
        load_widget_->update();
}

bool DataSourceManager::loadDataSourcesFiltered()
{
    for (auto& ds_it : db_data_sources_)
        if (!ds_it->loadingWanted())
            return true;

    return false;
}

std::set<unsigned int> DataSourceManager::getLoadDataSources ()
{
    std::set<unsigned int> ds_to_load;

    for (auto& ds_it : db_data_sources_)
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted())
            ds_to_load.insert(ds_it->id());

    return ds_to_load;
}

void DataSourceManager::databaseOpenedSlot()
{

    loginf << "DataSourceManager: databaseOpenedSlot";

    loadDBDataSources();

    if (load_widget_)
        load_widget_->update();
}

void DataSourceManager::databaseClosedSlot()
{
    db_data_sources_.clear();

    updateDSIdsAll();

    if (load_widget_)
        load_widget_->update();
}

void DataSourceManager::configurationDialogDoneSlot()
{
    loginf << "DataSourceManager: configurationDialogDoneSlot";

    config_dialog_->hide();
    config_dialog_ = nullptr;

}

bool DataSourceManager::hasConfigDataSource (unsigned int ds_id)
{
    return find_if(config_data_sources_.begin(), config_data_sources_.end(),
                   [ds_id] (const std::unique_ptr<dbContent::ConfigurationDataSource>& s)
    { return s->id() == ds_id; } ) != config_data_sources_.end();
}

dbContent::ConfigurationDataSource& DataSourceManager::configDataSource (unsigned int ds_id)
{
    assert (hasConfigDataSource(ds_id));

    return *find_if(config_data_sources_.begin(), config_data_sources_.end(),
                    [ds_id] (const std::unique_ptr<dbContent::ConfigurationDataSource>& s)
    { return s->id() == ds_id; } )->get();
}

void DataSourceManager::checkSubConfigurables()
{

}

void DataSourceManager::loadDBDataSources()
{
    assert (!db_data_sources_.size());

    DBInterface& db_interface = COMPASS::instance().interface();

    if (db_interface.existsDataSourcesTable())
    {
        db_data_sources_ = db_interface.getDataSources();
        sortDBDataSources();
    }
}

void DataSourceManager::sortDBDataSources()
{
    sort(db_data_sources_.begin(), db_data_sources_.end(),
         [](const std::unique_ptr<dbContent::DBDataSource>& a,
         const std::unique_ptr<dbContent::DBDataSource>& b) -> bool
    {
        return a->name() < b->name();
    });
}

void DataSourceManager::updateDSIdsAll()
{
    ds_ids_all_.clear();

    std::set<unsigned int> ds_ids_set;

    for (auto& ds_it : config_data_sources_) // add from cfg
    {
        if (!ds_ids_set.count(ds_it->id()))
            ds_ids_set.insert(ds_it->id());
    }

    for (auto& ds_it : db_data_sources_) // add from db
    {
        if (!ds_ids_set.count(ds_it->id()))
            ds_ids_set.insert(ds_it->id());
    }

    std::copy(ds_ids_set.begin(), ds_ids_set.end(), std::back_inserter(ds_ids_all_)); // copy to vec
}

void DataSourceManager::saveDBDataSources()
{
    DBInterface& db_interface = COMPASS::instance().interface();

    assert(db_interface.dbOpen());
    db_interface.saveDataSources(db_data_sources_);
}

bool DataSourceManager::canAddNewDataSourceFromConfig (unsigned int ds_id)
{
    if (hasDBDataSource(ds_id))
        return false;

    return hasConfigDataSource(ds_id);
}

bool DataSourceManager::hasDBDataSource(unsigned int ds_id)
{
    return find_if(db_data_sources_.begin(), db_data_sources_.end(),
                   [ds_id] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return s->id() == ds_id; } ) != db_data_sources_.end();
}

bool DataSourceManager::hasDataSourcesOfDBContent(const std::string dbcontent_name)
{
    return find_if(db_data_sources_.begin(), db_data_sources_.end(),
                   [dbcontent_name] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return s->numInsertedMap().count(dbcontent_name) > 0; } ) != db_data_sources_.end();
}


void DataSourceManager::addNewDataSource (unsigned int ds_id)
{
    loginf << "DataSourceManager: addNewDataSource: ds_id " << ds_id;

    assert (!hasDBDataSource(ds_id));

    if (hasConfigDataSource(ds_id))
    {
        loginf << "DataSourceManager: addNewDataSource: ds_id " << ds_id << " from config";

        dbContent::ConfigurationDataSource& cfg_ds = configDataSource(ds_id);

        db_data_sources_.emplace_back(move(cfg_ds.getAsNewDBDS()));
        sortDBDataSources();
    }
    else
    {
        loginf << "DataSourceManager: addNewDataSource: ds_id " << ds_id << " create new";

        dbContent::DBDataSource* new_ds = new dbContent::DBDataSource();
        new_ds->id(ds_id);
        new_ds->sac(Number::sacFromDsId(ds_id));
        new_ds->sic(Number::sicFromDsId(ds_id));
        new_ds->name(to_string(ds_id));

        db_data_sources_.emplace_back(move(new_ds));
        sortDBDataSources();
    }

    assert (hasDBDataSource(ds_id));
    updateDSIdsAll();

    loginf << "DataSourceManager: addNewDataSource: ds_id " << ds_id << " done";
}

dbContent::DBDataSource& DataSourceManager::dbDataSource(unsigned int ds_id)
{
    assert (hasDBDataSource(ds_id));

    return *find_if(db_data_sources_.begin(), db_data_sources_.end(),
                    [ds_id] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return Number::dsIdFrom(s->sac(), s->sic()) == ds_id; } )->get();
}

bool DataSourceManager::dsTypeFiltered()
{
    for (auto& ds_type_it : ds_type_loading_wanted_)
        if (!ds_type_it.second)
            return true;

    return false;
}

std::set<std::string> DataSourceManager::wantedDSTypes()
{
    std::set<std::string> ret;

    for (auto& ds_type_it : ds_type_loading_wanted_)
        if (ds_type_it.second)
            ret.insert(ds_type_it.first);

    return ret;
}

void DataSourceManager::dsTypeLoadingWanted (const std::string& ds_type, bool wanted)
{
    loginf << "DataSourceManager: dsTypeLoadingWanted: ds_type " << ds_type << " wanted " << wanted;

    ds_type_loading_wanted_[ds_type] = wanted;
}

bool DataSourceManager::dsTypeLoadingWanted (const std::string& ds_type)
{
    if (!ds_type_loading_wanted_.count(ds_type))
        return true;

    return ds_type_loading_wanted_.at(ds_type);
}

void DataSourceManager::setLoadDSTypes (bool loading_wanted)
{
    loginf << "DataSourceManager: setLoadDSTypes: wanted " << loading_wanted;

    for (auto& ds_type_it : data_source_types_)
    {
        logdbg << "DataSourceManager: setLoadDSTypes: wanted " << loading_wanted;
        ds_type_loading_wanted_[ds_type_it] = loading_wanted;
    }

    if (load_widget_)
        load_widget_->update();
}

void DataSourceManager::setLoadOnlyDSTypes (std::set<std::string> ds_types)
{
    // deactivate all loading
    setLoadDSTypes(false);

    for (auto ds_type_it : ds_types)
    {
        dsTypeLoadingWanted(ds_type_it, true);
    }

    if (load_widget_)
        load_widget_->update();
}

const std::vector<std::unique_ptr<dbContent::DBDataSource>>& DataSourceManager::dbDataSources() const
{
    return db_data_sources_;
}

std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> DataSourceManager::getNetworkLines()
{
    //ds_id -> (ip, port)
    std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> lines;

    string line_address;
    string ip;
    unsigned int port;

    set<string> existing_lines; // to check

    for (auto& ds_it : config_data_sources_)
    {
        if (ds_it->info().contains("network_lines"))
        {
            json& network_lines = ds_it->info().at("network_lines");
            assert (network_lines.is_array());

            for (auto& line_it : network_lines.get<json::array_t>())  // iterate over array
            {
                assert (line_it.is_primitive());
                assert (line_it.is_string());

                line_address = line_it;

                ip = String::ipFromString(line_address);
                port = String::portFromString(line_address);

                if (existing_lines.count(ip+":"+to_string(port)))
                {
                    logwrn << "DataSourceManager: getNetworkLines: source " << ds_it->name()
                           << " line " << ip << ":" << port
                           << " already in use";
                }
                else
                    lines[Number::dsIdFrom(ds_it->sac(), ds_it->sic())].push_back({ip, port});

                existing_lines.insert(ip+":"+to_string(port));

                //break; // TODO only parse one for now
            }
        }
    }

//    for (auto& ds_it : db_data_sources_) // should be same
//    {
//        if (ds_it->info().contains("network_lines"))
//        {
//            json& network_lines = ds_it->info().at("network_lines");
//            assert (network_lines.is_array());

//            for (auto& line_it : network_lines.get<json::array_t>())  // iterate over array
//            {
//                assert (line_it.is_primitive());
//                assert (line_it.is_string());

//                line_address = line_it;

//                ip = String::ipFromString(line_address);
//                port = String::portFromString(line_address);

//                lines[Number::dsIdFrom(ds_it->sac(), ds_it->sic())].push_back({ip, port});
//            }
//        }
//    }

    return lines;
}
