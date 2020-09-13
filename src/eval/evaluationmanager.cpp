#include "evaluationmanager.h"
#include "evaluationmanagerwidget.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "sector.h"

#include "json.hpp"

#include <QTabWidget>

#include <memory>
#include <fstream>

//using namespace Utils;
using namespace std;
using namespace nlohmann;

EvaluationManager::EvaluationManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb)
    : Configurable(class_id, instance_id, atsdb, "eval.json"), atsdb_(*atsdb)
{
    registerParameter("dbo_name_ref", &dbo_name_ref_, "");
    registerParameter("active_sources_ref", &active_sources_ref_, json::object());

    updateReferenceDBO();

    registerParameter("dbo_name_tst", &dbo_name_tst_, "");
    registerParameter("active_sources_tst", &active_sources_tst_, json::object());

    updateTestDBO();

    createSubConfigurables();
}

void EvaluationManager::init(QTabWidget* tab_widget)
{
    loginf << "EvaluationManager: init";

    assert (!initialized_);
    assert (tab_widget);

    loadSectors();

    initialized_ = true;

    tab_widget->addTab(widget(), "Evaluation");
}

void EvaluationManager::close()
{
    initialized_ = false;
}

EvaluationManager::~EvaluationManager()
{
    sector_layers_.clear();
}

void EvaluationManager::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    throw std::runtime_error("EvaluationManager: generateSubConfigurable: unknown class_id " +
                             class_id);
}

EvaluationManagerWidget* EvaluationManager::widget()
{
    if (!widget_)
    {
        widget_.reset(new EvaluationManagerWidget(*this));
    }

    assert(widget_);
    return widget_.get();
}

void EvaluationManager::checkSubConfigurables()
{

}

bool EvaluationManager::hasSectorLayer (const std::string& layer_name)
{
    assert (initialized_);

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

    return iter != sector_layers_.end();
}

//void EvaluationManager::renameSectorLayer (const std::string& name, const std::string& new_name)
//{
//    // TODO
//}

std::shared_ptr<SectorLayer> EvaluationManager::sectorLayer (const std::string& layer_name)
{
    assert (initialized_);
    assert (hasSectorLayer(layer_name));

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});
    assert (iter != sector_layers_.end());

    return *iter;
}

void EvaluationManager::loadSectors()
{
    loginf << "EvaluationManager: loadSectors";

    assert (!initialized_);

    sector_layers_ = ATSDB::instance().interface().loadSectors();
}

unsigned int EvaluationManager::getMaxSectorId ()
{
    assert (initialized_);

    unsigned int id = 0;
    for (auto& sec_lay_it : sector_layers_)
        for (auto& sec_it : sec_lay_it->sectors())
            if (sec_it->id() > id)
                id = sec_it->id();

    return id;
}

void EvaluationManager::createNewSector (const std::string& name, const std::string& layer_name,
                                   std::vector<std::pair<double,double>> points)
{
    loginf << "EvaluationManager: createNewSector: name " << name << " layer_name " << layer_name
           << " num points " << points.size();

    assert (initialized_);
    assert (!hasSector(name, layer_name));

    unsigned int id = getMaxSectorId()+1;

    shared_ptr<Sector> sector = make_shared<Sector> (id, name, layer_name, points);

    // add to existing sectors
    if (!hasSectorLayer(layer_name))
        sector_layers_.push_back(make_shared<SectorLayer>(layer_name));

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->addSector(sector);

    assert (hasSector(name, layer_name));
    sector->save();
}

bool EvaluationManager::hasSector (const string& name, const string& layer_name)
{
    assert (initialized_);

    if (!hasSectorLayer(layer_name))
        return false;

    return sectorLayer(layer_name)->hasSector(name);
}

bool EvaluationManager::hasSector (unsigned int id)
{
    assert (initialized_);

    for (auto& sec_lay_it : sector_layers_)
    {
        auto& sectors = sec_lay_it->sectors();
        auto iter = std::find_if(sectors.begin(), sectors.end(),
                                 [id](const shared_ptr<Sector>& x) { return x->id() == id;});
        if (iter != sectors.end())
            return true;
    }
    return false;
}

std::shared_ptr<Sector> EvaluationManager::sector (const string& name, const string& layer_name)
{
    assert (initialized_);
    assert (hasSector(name, layer_name));

    return sectorLayer(layer_name)->sector(name);
}

std::shared_ptr<Sector> EvaluationManager::sector (unsigned int id)
{
    assert (initialized_);
    assert (hasSector(id));

    for (auto& sec_lay_it : sector_layers_)
    {
        auto& sectors = sec_lay_it->sectors();
        auto iter = std::find_if(sectors.begin(), sectors.end(),
                                 [id](const shared_ptr<Sector>& x) { return x->id() == id;});
        if (iter != sectors.end())
            return *iter;
    }

    logerr << "EvaluationManager: sector: id " << id << " not found";
    assert (false);
}

void EvaluationManager::moveSector(unsigned int id, const std::string& old_layer_name, const std::string& new_layer_name)
{
    assert (initialized_);
    assert (hasSector(id));

    shared_ptr<Sector> tmp_sector = sector(id);

    assert (hasSectorLayer(old_layer_name));
    sectorLayer(old_layer_name)->removeSector(tmp_sector);

    if (!hasSectorLayer(new_layer_name))
        sector_layers_.push_back(make_shared<SectorLayer>(new_layer_name));

    assert (hasSectorLayer(new_layer_name));
    sectorLayer(new_layer_name)->addSector(tmp_sector);

    assert (hasSector(tmp_sector->name(), new_layer_name));
    tmp_sector->save();
}

std::vector<std::shared_ptr<SectorLayer>>& EvaluationManager::sectorsLayers()
{
    assert (initialized_);

    return sector_layers_;
}

void EvaluationManager::saveSector(unsigned int id)
{
    assert (initialized_);
    assert (hasSector(id));

    saveSector(sector(id));
}

void EvaluationManager::saveSector(std::shared_ptr<Sector> sector)
{
    assert (initialized_);
    assert (hasSector(sector->name(), sector->layerName()));
    ATSDB::instance().interface().saveSector(sector);

    emit sectorsChangedSignal();
}

void EvaluationManager::deleteSector(shared_ptr<Sector> sector)
{
    assert (initialized_);
    assert (hasSector(sector->name(), sector->layerName()));

    string layer_name = sector->layerName();

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->removeSector(sector);

    // remove sector layer if empty
    if (!sectorLayer(layer_name)->size())
    {
        auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                                 [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

        assert (iter != sector_layers_.end());
        sector_layers_.erase(iter);
    }

    ATSDB::instance().interface().deleteSector(sector);

    emit sectorsChangedSignal();
}

void EvaluationManager::deleteAllSectors()
{
    assert (initialized_);
    sector_layers_.clear();

    ATSDB::instance().interface().deleteAllSectors();

    emit sectorsChangedSignal();
}


void EvaluationManager::importSectors (const std::string& filename)
{
    loginf << "EvaluationManager: importSectors: filename '" << filename << "'";

    assert (initialized_);

    sector_layers_.clear();
    ATSDB::instance().interface().clearSectorsTable();

    std::ifstream input_file(filename, std::ifstream::in);

    try
    {
        json j = json::parse(input_file);

        if (!j.contains("sectors"))
        {
            logerr << "EvaluationManager: importSectors: file does not contain sectors";
            return;
        }

        json& sectors = j["sectors"];

        if (!sectors.is_array())
        {
            logerr << "EvaluationManager: importSectors: file sectors is not an array";
            return;
        }

        unsigned int id;
        string name;
        string layer_name;
        string json_str;

        for (auto& j_sec_it : sectors.get<json::array_t>())
        {
            if (!j_sec_it.contains("id")
                    || !j_sec_it.contains("name")
                    || !j_sec_it.contains("layer_name")
                    || !j_sec_it.contains("points"))
            {
                logerr << "EvaluationManager: importSectors: ill-defined sectors skipped, json '" << j_sec_it.dump(4)
                       << "'";
                continue;
            }

            id = j_sec_it.at("id");
            name = j_sec_it.at("name");
            layer_name = j_sec_it.at("layer_name");

            shared_ptr<Sector> new_sector = make_shared<Sector>(id, name, layer_name, j_sec_it.dump());

            if (!hasSectorLayer(layer_name))
                sector_layers_.push_back(make_shared<SectorLayer>(layer_name));

            sectorLayer(layer_name)->addSector(new_sector);

            assert (hasSector(name, layer_name));

            new_sector->save();

            loginf << "EvaluationManager: importSectors: loaded sector '" << name << "' in layer '"
                   << layer_name << "' num points " << sector(name, layer_name)->size();
        }
    }
    catch (json::exception& e)
    {
        logerr << "EvaluationManager: importSectors: could not load file '"
               << filename << "'";
        throw e;
    }

    emit sectorsChangedSignal();
}

void EvaluationManager::exportSectors (const std::string& filename)
{
    loginf << "EvaluationManager: exportSectors: filename '" << filename << "'";

    assert (initialized_);

    json j;

    j["sectors"] = json::array();
    json& sectors = j["sectors"];

    unsigned int cnt = 0;

    for (auto& sec_lay_it : sector_layers_)
    {
        for (auto& sec_it : sec_lay_it->sectors())
        {
            sectors[cnt] = sec_it->jsonData();
            ++cnt;
        }
    }

    std::ofstream output_file;
    output_file.open(filename, std::ios_base::out);

    output_file << j.dump(4);

}

std::string EvaluationManager::dboNameRef() const
{
    return dbo_name_ref_;
}

void EvaluationManager::dboNameRef(const std::string& name)
{
    loginf << "EvaluationManager: dboNameRef: name " << name;

    dbo_name_ref_ = name;

    updateReferenceDBO();
}

bool EvaluationManager::hasValidReferenceDBO ()
{
    if (!dbo_name_ref_.size())
        return false;

    if (!ATSDB::instance().objectManager().existsObject(dbo_name_ref_))
        return false;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    if (!object.hasCurrentDataSourceDefinition())
        return false;

    return object.hasDataSources();
}

std::string EvaluationManager::dboNameTst() const
{
    return dbo_name_tst_;
}

void EvaluationManager::dboNameTst(const std::string& name)
{
    loginf << "EvaluationManager: dboNameTst: name " << name;

    dbo_name_tst_ = name;

    updateTestDBO();
}

bool EvaluationManager::hasValidTestDBO ()
{
    if (!dbo_name_tst_.size())
        return false;

    if (!ATSDB::instance().objectManager().existsObject(dbo_name_tst_))
        return false;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    if (!object.hasCurrentDataSourceDefinition())
        return false;

    return object.hasDataSources();
}

void EvaluationManager::updateReferenceDBO()
{
    loginf << "EvaluationManager: updateReferenceDBO";

    data_sources_ref_.clear();
    active_sources_ref_.clear();

    if (!hasValidReferenceDBO())
        return;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    if (object.hasDataSources())
        updateReferenceDataSources();

    if (object.hasActiveDataSourcesInfo())
        updateReferenceDataSourcesActive();
}

void EvaluationManager::updateReferenceDataSources()
{
    loginf << "EvaluationManager: updateReferenceDataSources";

    assert (hasValidReferenceDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    for (auto ds_it = object.dsBegin(); ds_it != object.dsEnd(); ++ds_it)
    {
        if (data_sources_ref_.find(ds_it->first) == data_sources_ref_.end())
        {
            if (!active_sources_ref_.contains(to_string(ds_it->first)))
                active_sources_ref_[to_string(ds_it->first)] = true; // init with default true

            // needed for old compiler
            json::boolean_t& active = active_sources_ref_[to_string(ds_it->first)].get_ref<json::boolean_t&>();

            data_sources_ref_.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(ds_it->first),  // args for key
                                  std::forward_as_tuple(ds_it->first, ds_it->second.name(),
                                                        active));
        }
    }
}

void EvaluationManager::updateReferenceDataSourcesActive()
{
    loginf << "EvaluationManager: updateReferenceDataSourcesActive";

    assert (hasValidReferenceDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_ref_);

    assert (object.hasActiveDataSourcesInfo());

    for (auto& srcit : data_sources_ref_)
        srcit.second.setActiveInData(false);

    for (auto& it : object.getActiveDataSources())
    {
        assert(data_sources_ref_.find(it) != data_sources_ref_.end());
        ActiveDataSource& src = data_sources_ref_.at(it);
        src.setActiveInData(true);
    }

    for (auto& srcit : data_sources_ref_)
    {
        if (!srcit.second.isActiveInData())
            srcit.second.setActive(false);
    }
}

void EvaluationManager::updateTestDBO()
{
    loginf << "EvaluationManager: updateTestDBO";

    data_sources_ref_.clear();
    active_sources_ref_.clear();

    if (!hasValidTestDBO())
        return;

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    if (object.hasDataSources())
        updateTestDataSources();

    if (object.hasActiveDataSourcesInfo())
        updateTestDataSourcesActive();
}

void EvaluationManager::updateTestDataSources()
{
    loginf << "EvaluationManager: updateTestDataSources";

    assert (hasValidTestDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    for (auto ds_it = object.dsBegin(); ds_it != object.dsEnd(); ++ds_it)
    {
        if (data_sources_tst_.find(ds_it->first) == data_sources_tst_.end())
        {
            if (!active_sources_tst_.contains(to_string(ds_it->first)))
                active_sources_tst_[to_string(ds_it->first)] = true; // init with default true

            // needed for old compiler
            json::boolean_t& active = active_sources_tst_[to_string(ds_it->first)].get_ref<json::boolean_t&>();

            data_sources_tst_.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(ds_it->first),  // args for key
                                  std::forward_as_tuple(ds_it->first, ds_it->second.name(),
                                                        active));
        }
    }
}

void EvaluationManager::updateTestDataSourcesActive()
{
    loginf << "EvaluationManager: updateTestDataSourcesActive";

    assert (hasValidTestDBO());

    DBObject& object = ATSDB::instance().objectManager().object(dbo_name_tst_);

    assert (object.hasActiveDataSourcesInfo());

    for (auto& srcit : data_sources_tst_)
        srcit.second.setActiveInData(false);

    for (auto& it : object.getActiveDataSources())
    {
        assert(data_sources_tst_.find(it) != data_sources_tst_.end());
        ActiveDataSource& src = data_sources_tst_.at(it);
        src.setActiveInData(true);
    }

    for (auto& srcit : data_sources_tst_)
    {
        if (!srcit.second.isActiveInData())
            srcit.second.setActive(false);
    }
}
