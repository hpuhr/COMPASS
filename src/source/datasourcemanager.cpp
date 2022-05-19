#include "datasourcemanager.h"
#include "datasourcesloadwidget.h"
#include "datasourcesconfigurationdialog.h"
#include "compass.h"
#include "dbinterface.h"
#include "stringconv.h"
#include "number.h"
#include "files.h"
#include "json.hpp"

#include <QMessageBox>

#include <fstream>

using namespace std;
using namespace Utils;
using namespace dbContent;
using namespace nlohmann;

const std::vector<std::string> DataSourceManager::data_source_types_ {"Radar", "MLAT", "ADSB", "Tracker", "RefTraj",
                                                                      "Other"};

DataSourceManager::DataSourceManager(const std::string& class_id, const std::string& instance_id,
                                     COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "data_sources.json"), compass_(*compass)
{
    registerParameter("load_widget_show_counts", &load_widget_show_counts_, true);
    registerParameter("load_widget_show_lines", &load_widget_show_lines_, true);

    createSubConfigurables();

    updateDSIdsAll();
}

DataSourceManager::~DataSourceManager()
{
    loginf << "DataSourceManager: dtor";

    config_data_sources_.clear();
    db_data_sources_.clear(); // delete their widgets, which removes them from load_widget_

    load_widget_ = nullptr;
}

void DataSourceManager::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "ConfigurationDataSource")
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
        load_widget_->updateContent();
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

void DataSourceManager::importDataSources(const std::string& filename)
{
    loginf << "DataSourceManager: importDataSources: file '" << filename << "'";

    try
    {
        if (!Files::fileExists(filename))
            throw std::runtime_error ("File '"+filename+"' not found.");

        std::ifstream input_file(filename, std::ifstream::in);

        json j = json::parse(input_file);

        if (!j.contains("content_type"))
            importDataSourcesJSONDeprecated(j);
        else
            importDataSourcesJSON(j);
    }
    catch (json::exception& e)
    {
        logerr << "DataSourceManager: importDataSources: could not load file '"
                   << filename << "'";
        throw e;
    }

    updateDSIdsAll();
    updateWidget();

    emit dataSourcesChangedSignal();
}

void DataSourceManager::importDataSourcesJSONDeprecated(const nlohmann::json& j)
{
    loginf << "DataSourceManager: importDataSourcesJSONDeprecated";

    for (auto& j_dbo_it : j.items())
    {
        std::string dbo_name = j_dbo_it.key();

        for (auto& j_ds_it : j_dbo_it.value().get<json::array_t>())
        {
            loginf << "DataSourceManager: importDataSources: found dbo " << dbo_name
                   << " ds '" << j_ds_it.dump(4) << "'";

            assert(j_ds_it.contains("dbo_name"));
            assert(j_ds_it.contains("name"));
            assert(j_ds_it.contains("sac"));
            assert(j_ds_it.contains("sic"));

            unsigned int sac = j_ds_it.at("sac");
            unsigned int sic = j_ds_it.at("sic");

            unsigned int ds_id = Number::dsIdFrom(sac, sic);

            if (!hasConfigDataSource(ds_id))
                createConfigDataSource(ds_id);

            configDataSource(ds_id).setFromJSONDeprecated(j_ds_it);

            if (hasDBDataSource(ds_id))
                dbDataSource(ds_id).setFromJSONDeprecated(j_ds_it);
        }
    }
}

void DataSourceManager::importDataSourcesJSON(const nlohmann::json& j)
{
    loginf << "DataSourceManager: importDataSourcesJSON";

    if (!j.contains("content_type")
            || !j.at("content_type").is_string()
            || j.at("content_type") != "data_sources")
        throw std::runtime_error("current data is not view point content");

    if (!j.contains("content_version")
            || !j.at("content_version").is_string()
            || j.at("content_version") != "0.2")
        throw std::runtime_error("current data content version is not supported");

    if (!j.contains("data_sources")
            || !j.at("data_sources").is_array())
        throw std::runtime_error("current data contains no data sources");

    for (auto& j_ds_it : j.at("data_sources").get<json::array_t>())
    {

        assert(j_ds_it.contains("ds_type"));
        assert(j_ds_it.contains("name"));
        assert(j_ds_it.contains("sac"));
        assert(j_ds_it.contains("sic"));

        unsigned int sac = j_ds_it.at("sac");
        unsigned int sic = j_ds_it.at("sic");

        unsigned int ds_id = Number::dsIdFrom(sac, sic);

        if (!hasConfigDataSource(ds_id))
            createConfigDataSource(ds_id);

        configDataSource(ds_id).setFromJSON(j_ds_it);

        if (hasDBDataSource(ds_id))
            dbDataSource(ds_id).setFromJSON(j_ds_it);
    }

    if (COMPASS::instance().dbOpened())
        saveDBDataSources();
}

void DataSourceManager::deleteAllConfigDataSources()
{
    loginf << "DataSourceManager: deleteAllConfigDataSources";

    for(auto it = config_data_sources_.begin(); it != config_data_sources_.end();)
    {
        if(!hasDBDataSource((*it)->id())) // erase if not in db
        {
            it = config_data_sources_.erase(it); // erase and update it
        }
        else // next one
        {
            ++it;
        }
    }

    updateDSIdsAll();

    emit dataSourcesChangedSignal();
}

void DataSourceManager::exportDataSources(const std::string& filename)
{
    loginf << "DataSourceManager: exportDataSources: file '" << filename << "'";

    json data;

    data["content_type"] = "data_sources";
    data["content_version"] = "0.2";

    data["data_sources"] = json::array();
    json& data_sources = data.at("data_sources");

    unsigned int cnt = 0;
    for (auto& ds_it : config_data_sources_)
    {
        data_sources[cnt] = ds_it->getAsJSON();
        ++cnt;
    }

    std::ofstream file(filename);
    file << data.dump(4);

    QMessageBox m_info(QMessageBox::Information, "Export Data Sources",
                       "File export: '"+QString(filename.c_str())+"' done.\n"
                       +QString::number(config_data_sources_.size())+" Data Sources saved.", QMessageBox::Ok);
    m_info.exec();
}

bool DataSourceManager::loadWidgetShowCounts() const
{
    return load_widget_show_counts_;
}

void DataSourceManager::loadWidgetShowCounts(bool value)
{
    loginf << "DataSourceManager: loadWidgetShowCounts: value " << value;

    load_widget_show_counts_ = value;

    if (load_widget_)
        load_widget_->updateContent();
}

bool DataSourceManager::loadWidgetShowLines() const
{
    return load_widget_show_lines_;
}

void DataSourceManager::loadWidgetShowLines(bool value)
{
    loginf << "DataSourceManager: loadWidgetShowLines: value " << value;

    load_widget_show_lines_ = value;

    if (load_widget_)
        load_widget_->updateContent();
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
    //assert (hasDSFilter(dbcontent_name)); can also be used if no filter active

    std::vector<unsigned int> ds_ids;

    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted() && ds_it->hasNumInserted(dbcontent_name))
            ds_ids.push_back(ds_it->id());
    }

    assert (ds_ids.size());

    return ds_ids;
}

bool DataSourceManager::lineSpecificLoadingRequired(const std::string& dbcontent_name)
{
    for (auto& ds_it : db_data_sources_)
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted() && ds_it->hasNumInserted(dbcontent_name)
                && ds_it->lineSpecificLoadingWanted())
            return true;

    return false;
}

void DataSourceManager::setLoadDataSources (bool loading_wanted)
{
    loginf << "DataSourceManager: setLoadDataSources: wanted " << loading_wanted;

    for (auto& ds_it : db_data_sources_)
        ds_it->loadingWanted(loading_wanted);

    if (load_widget_)
        load_widget_->updateContent();
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
        load_widget_->updateContent();
}

bool DataSourceManager::loadDataSourcesFiltered()
{
    for (auto& ds_it : db_data_sources_)
        if (!ds_it->loadingWanted())
            return true;

    return false;
}

std::map<unsigned int, std::set<unsigned int>> DataSourceManager::getLoadDataSources ()
{
    std::map<unsigned int, std::set<unsigned int>> ds_to_load;

    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted())
            ds_to_load[ds_it->id()] = ds_it->getLoadingWantedLines();
    }

    return ds_to_load;
}

void DataSourceManager::databaseOpenedSlot()
{

    loginf << "DataSourceManager: databaseOpenedSlot";

    loadDBDataSources();

    emit dataSourcesChangedSignal();

    if (load_widget_)
        load_widget_->updateContent();
}

void DataSourceManager::databaseClosedSlot()
{
    db_data_sources_.clear();

    updateDSIdsAll();

    emit dataSourcesChangedSignal();

    if (load_widget_)
        load_widget_->updateContent();
}

void DataSourceManager::configurationDialogDoneSlot()
{
    loginf << "DataSourceManager: configurationDialogDoneSlot";

    config_dialog_->hide();
    config_dialog_ = nullptr;

    emit dataSourcesChangedSignal();
}

bool DataSourceManager::hasConfigDataSource (unsigned int ds_id)
{
    return find_if(config_data_sources_.begin(), config_data_sources_.end(),
                   [ds_id] (const std::unique_ptr<dbContent::ConfigurationDataSource>& s)
    { return s->id() == ds_id; } ) != config_data_sources_.end();
}

void DataSourceManager::createConfigDataSource(unsigned int ds_id)
{
    assert (!hasConfigDataSource(ds_id));

    Configuration& new_cfg = configuration().addNewSubConfiguration("ConfigurationDataSource");
    new_cfg.addParameterString("ds_type", "Other");
    new_cfg.addParameterUnsignedInt("sac", Number::sacFromDsId(ds_id));
    new_cfg.addParameterUnsignedInt("sic", Number::sicFromDsId(ds_id));
    new_cfg.addParameterString("name", "Unknown ("+to_string(Number::sacFromDsId(ds_id))+"/"
                                                  +to_string(Number::sicFromDsId(ds_id))+")");

    generateSubConfigurable("ConfigurationDataSource", new_cfg.getInstanceId());

    updateDSIdsAll();
}

void DataSourceManager::deleteConfigDataSource(unsigned int ds_id)
{
    loginf << "DataSourceManager: deleteConfigDataSource: ds_id " << ds_id;

    assert (hasConfigDataSource(ds_id));
    assert (!hasDBDataSource(ds_id)); // can not delete config data sources in existing db

    auto ds_it = find_if(config_data_sources_.begin(), config_data_sources_.end(),
                         [ds_id] (const std::unique_ptr<dbContent::ConfigurationDataSource>& s)
    { return s->id() == ds_id; } );

    config_data_sources_.erase(ds_it);

    updateDSIdsAll();

    emit dataSourcesChangedSignal();
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

bool DataSourceManager::hasDBDataSource(const std::string& ds_name)
{
    return find_if(db_data_sources_.begin(), db_data_sources_.end(),
                   [ds_name] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return (s->hasShortName() ? s->shortName() == ds_name : false)
                || s->name() == ds_name; } ) != db_data_sources_.end();
}

unsigned int DataSourceManager::getDBDataSourceDSID(const std::string& ds_name)
{
    auto ds_it = find_if(db_data_sources_.begin(), db_data_sources_.end(),
                         [ds_name] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return (s->hasShortName() ? s->shortName() == ds_name : false)
                || s->name() == ds_name; } );

    assert (ds_it != db_data_sources_.end());

    return (*ds_it)->id();
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

        createConfigDataSource(ds_id);

        dbContent::DBDataSource* new_ds = new dbContent::DBDataSource();
        new_ds->id(ds_id);
        new_ds->sac(Number::sacFromDsId(ds_id));
        new_ds->sic(Number::sicFromDsId(ds_id));
        new_ds->dsType("Other");
        new_ds->name("Unknown ("+to_string(Number::sacFromDsId(ds_id))+"/"
                                +to_string(Number::sicFromDsId(ds_id))+")");

        db_data_sources_.emplace_back(move(new_ds));
        sortDBDataSources();


    }

    assert (hasDBDataSource(ds_id));
    updateDSIdsAll();

    emit dataSourcesChangedSignal();

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
        load_widget_->updateContent();
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
        load_widget_->updateContent();
}

const std::vector<std::unique_ptr<dbContent::DBDataSource>>& DataSourceManager::dbDataSources() const
{
    return db_data_sources_;
}

void DataSourceManager::createNetworkDBDataSources()
{
    unsigned int ds_id;

    for (auto& ds_it : config_data_sources_)
    {
        ds_id = ds_it->id();

        if (ds_it->hasNetworkLines())
        {

            if (!hasDBDataSource(ds_id))
            {
                loginf << "DataSourceManager: createNetworkDBDataSources: ds_id " << ds_id << " from config";

                db_data_sources_.emplace_back(move(ds_it->getAsNewDBDS()));
                //addNewDataSource(ds_it->id());
            }

            unsigned int line_cnt;
            bool first = true;

            for (auto& line_it : ds_it->networkLines()) // lx -> ip, port
            {
                line_cnt = String::getAppendedInt(line_it.first);
                assert (line_cnt >= 0 && line_cnt <= 4);

                dbDataSource(ds_it->id()).lineLoadingWanted(line_cnt - 1, first); // only load first one
                //dbDataSource(ds_it->id()).addNumInserted()

                first = false;
            }
        }
    }

    sortDBDataSources();
    updateDSIdsAll();

    emit dataSourcesChangedSignal();
}

std::map<unsigned int, std::map<std::string, std::pair<std::string, unsigned int>>> DataSourceManager::getNetworkLines()
{
    // ds_id -> line str ->(ip, port)
    std::map<unsigned int, std::map<std::string, std::pair<std::string, unsigned int>>> lines;

    for (auto& ds_it : config_data_sources_)
    {
        if (ds_it->hasNetworkLines())
        {
            lines[ds_it->id()] = ds_it->networkLines();
        }
    }

    //db_data_sources_ // should be same

    return lines;
}
