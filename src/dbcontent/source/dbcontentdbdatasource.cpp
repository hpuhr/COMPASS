#include "dbcontentdbdatasource.h"
#include "logger.h"

using namespace nlohmann;

const std::string DBDataSource::table_name_{"data_sources"};

const Property DBDataSource::id_column_{"ds_id", PropertyDataType::UINT};
const Property DBDataSource::ds_type_column_{"ds_type", PropertyDataType::STRING};
const Property DBDataSource::sac_column_{"sac", PropertyDataType::UINT};
const Property DBDataSource::sic_column_{"sic", PropertyDataType::UINT};
const Property DBDataSource::name_column_{"name", PropertyDataType::STRING};
const Property DBDataSource::short_name_{"short_name", PropertyDataType::STRING};
const Property DBDataSource::info_column_{"info", PropertyDataType::STRING};
const Property DBDataSource::counts_column_{"counts", PropertyDataType::STRING};

DBDataSource::DBDataSource()
{

}

DBDataSource::~DBDataSource()
{

}

void DBDataSource::counts (const std::string& counts)
{
    counts_ = json::parse(counts);

    counts_map_ = counts_.get<std::map<std::string, std::map<unsigned int, unsigned int>>>();
}

std::string DBDataSource::countsStr()
{
    return counts_.dump();
}

bool DBDataSource::hasNumInserted() const
{
    return counts_map_.size();
}

bool DBDataSource::hasNumInserted(const std::string& db_content) const
{
    if (!counts_map_.count(db_content))
        return false;

    return counts_map_.at(db_content).size();
}

const std::map<std::string, std::map<unsigned int, unsigned int>>& DBDataSource::numInsertedMap() const
{
    assert (hasNumInserted());
    return counts_map_;
}

std::map<std::string, unsigned int> DBDataSource::numInsertedSummedLinesMap() const
{
    std::map<std::string, unsigned int> line_sum_map;

    for (auto& db_cont_it : counts_map_)
    {
        for (auto& cnt_it : db_cont_it.second)
        {
            line_sum_map[db_cont_it.first] += cnt_it.second;
        }
    }

    return line_sum_map;
}

void DBDataSource::addNumInserted(const std::string& db_content, unsigned int line_id, unsigned int num)
{
    counts_map_[db_content][line_id] += num;

    counts_ = counts_map_;
}

void DBDataSource::addNumLoaded(const std::string& db_content, unsigned int line_id, unsigned int num)
{
    num_loaded_[db_content][line_id] += num;
}

unsigned int DBDataSource::numLoaded (const std::string& db_content)
{
    unsigned int num_loaded = 0;

    if (num_loaded_.count(db_content))
    {
        for (auto& cnt_it : num_loaded_.at(db_content))
            num_loaded += cnt_it.second;
    }

    return num_loaded;
}

void DBDataSource::clearNumLoaded()
{
    num_loaded_.clear();
}

//json DBDataSource::getAsJSON()
//{
//    json j = DataSourceBase::getAsJSON();

//    j["counts"] = counts_;

//    return j;
//}

//void DBDataSource::setFromJSON(json& j)
//{
//    DataSourceBase::setFromJSON(j);

//    counts_ = j.at("counts");
//}

unsigned int DBDataSource::id() const
{
    return id_;
}

void DBDataSource::id(unsigned int id)
{
    id_ = id;
}

bool DBDataSource::loadingWanted() const
{
    return loading_wanted_;
}

void DBDataSource::loadingWanted(bool loading_wanted)
{
    loginf << "DBDataSource: loadingWanted: ds " << name_ << " wanted " << loading_wanted;

    loading_wanted_ = loading_wanted;
}


