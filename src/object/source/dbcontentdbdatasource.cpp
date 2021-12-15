#include "dbcontentdbdatasource.h"
#include "logger.h"

using namespace nlohmann;

namespace DBContent {

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

    counts_map_ = counts_.get<std::map<unsigned int, std::map<std::string, unsigned int>>>();
}

std::string DBDataSource::countsStr()
{
    return counts_.dump();
}

bool DBDataSource::hasActiveNumInserted() const
{
    return counts_map_.count(active_line_);
}

const std::map<std::string, unsigned int>& DBDataSource::activeNumInsertedMap() const
{
    assert (hasActiveNumInserted());
    return counts_map_.at(active_line_);
}

void DBDataSource::addNumInserted(unsigned int line_id, const std::string& db_content, unsigned int num)
{
    counts_map_[line_id][db_content] += num;

    counts_ = counts_map_;
}

void DBDataSource::addNumLoaded(unsigned int line_id, const std::string& db_content, unsigned int num)
{
    num_loaded_[line_id][db_content] += num;
}

unsigned int DBDataSource::activeNumLoaded (const std::string& db_content)
{
    if (num_loaded_.count(active_line_) && num_loaded_.at(active_line_).count(db_content))
        return num_loaded_.at(active_line_).at(db_content);
    else
        return 0;
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

unsigned int DBDataSource::activeLine() const
{
    return active_line_;
}

void DBDataSource::activeLine(unsigned int active_line)
{
    loginf << "DBDataSource: activeLine: num " << active_line;

    active_line_ = active_line;
}



} // namespace DBContent
